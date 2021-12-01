// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToStadiaWidget.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLayout>
#include <QMessageBox>
#include <QModelIndexList>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTableView>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>
#include <utility>

#include "MainThreadExecutor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitGgp/Project.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/Error.h"
#include "SessionSetup/OtherUserDialog.h"
#include "SessionSetup/OverlayWidget.h"
#include "SessionSetup/RetrieveInstancesWidget.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "ui_ConnectToStadiaWidget.h"

namespace {
const QString kRememberChosenInstance{"RememberChosenInstance"};
}  // namespace

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Account;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using orbit_ggp::SshInfo;
using orbit_ssh::Credentials;
using orbit_ssh_qt::ScopedConnection;

// The destructor needs to be defined here because it needs to see the type
// `Ui::ConnectToStadiaWidget`. The header file only contains a forward declaration.
ConnectToStadiaWidget::~ConnectToStadiaWidget() = default;

ConnectToStadiaWidget::ConnectToStadiaWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::ConnectToStadiaWidget>()),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()),
      s_idle_(&state_machine_),
      s_instances_loading_(&state_machine_),
      s_instance_selected_(&state_machine_),
      s_loading_credentials_(&state_machine_),
      s_deploying_(&state_machine_),
      s_connected_(&state_machine_) {
  ui_->setupUi(this);
  ui_->instancesTableOverlay->raise();

  QSettings settings;
  if (settings.contains(kRememberChosenInstance)) {
    remembered_instance_id_ = settings.value(kRememberChosenInstance).toString();
    ui_->rememberCheckBox->setChecked(true);
  }

  instance_proxy_model_.setSourceModel(&instance_model_);
  instance_proxy_model_.setSortRole(Qt::DisplayRole);
  instance_proxy_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);
  // -1 means to filter based on *all* columns
  // (https://doc.qt.io/qt-5/qsortfilterproxymodel.html#filterKeyColumn-prop)
  instance_proxy_model_.setFilterKeyColumn(-1);

  ui_->instancesTableView->setModel(&instance_proxy_model_);
  ui_->instancesTableView->setSortingEnabled(true);
  ui_->instancesTableView->sortByColumn(
      static_cast<int>(orbit_ggp::InstanceItemModel::Columns::kDisplayName),
      Qt::SortOrder::AscendingOrder);

  QObject::connect(ui_->radioButton, &QRadioButton::clicked, this,
                   &ConnectToStadiaWidget::OnConnectToStadiaRadioButtonClicked);
  QObject::connect(this, &ConnectToStadiaWidget::ErrorOccurred, this,
                   &ConnectToStadiaWidget::OnErrorOccurred);
  QObject::connect(ui_->instancesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, &ConnectToStadiaWidget::OnSelectionChanged);
  QObject::connect(ui_->rememberCheckBox, &QCheckBox::toggled, this,
                   &ConnectToStadiaWidget::UpdateRememberInstance);
  QObject::connect(ui_->retrieveInstancesWidget, &RetrieveInstancesWidget::LoadingSuccessful, this,
                   &ConnectToStadiaWidget::OnInstancesLoaded);
  QObject::connect(ui_->retrieveInstancesWidget, &RetrieveInstancesWidget::FilterTextChanged,
                   &instance_proxy_model_, &QSortFilterProxyModel::setFilterFixedString);
  QObject::connect(ui_->connectButton, &QPushButton::clicked, this,
                   &ConnectToStadiaWidget::Connect);
  QObject::connect(ui_->instancesTableView, &QTableView::doubleClicked, this,
                   &ConnectToStadiaWidget::Connect);

  SetupStateMachine();
}

void ConnectToStadiaWidget::SetActive(bool value) {
  ui_->contentFrame->setEnabled(value);
  ui_->radioButton->setChecked(value);
}

void ConnectToStadiaWidget::SetSshConnectionArtifacts(
    SshConnectionArtifacts* ssh_connection_artifacts) {
  CHECK(ssh_connection_artifacts != nullptr);
  ssh_connection_artifacts_ = ssh_connection_artifacts;
}

void ConnectToStadiaWidget::SetConnection(StadiaConnection connection) {
  selected_instance_ = std::move(connection.instance_);
  service_deploy_manager_ = std::move(connection.service_deploy_manager_);
  grpc_channel_ = std::move(connection.grpc_channel_);

  QObject::connect(
      service_deploy_manager_.get(), &ServiceDeployManager::socketErrorOccurred, this,
      [this](std::error_code error) {
        emit ErrorOccurred(QString("The connection to instance %1 failed with error: %2")
                               .arg(selected_instance_->display_name)
                               .arg(QString::fromStdString(error.message())));
      });
}

void ConnectToStadiaWidget::Start() {
  CHECK(ssh_connection_artifacts_ != nullptr);

  auto client_result = orbit_ggp::CreateClient();
  if (client_result.has_error()) {
    ui_->radioButton->setToolTip(QString::fromStdString(client_result.error().message()));
    setEnabled(false);
    return;
  }
  ggp_client_ = std::move(client_result.value());

  if (grpc_channel_ != nullptr && grpc_channel_->GetState(false) == GRPC_CHANNEL_READY) {
    state_machine_.setInitialState(&s_connected_);
  } else {
    state_machine_.setInitialState(&s_instances_loading_);
  }

  state_machine_.start();

  retrieve_instances_ = RetrieveInstances::Create(ggp_client_.get(), main_thread_executor_.get());
  ui_->retrieveInstancesWidget->SetRetrieveInstances(retrieve_instances_.get());
  ui_->retrieveInstancesWidget->Start();
}

std::optional<StadiaConnection> ConnectToStadiaWidget::StopAndClearConnection() {
  if (!selected_instance_.has_value() || service_deploy_manager_ == nullptr ||
      grpc_channel_ == nullptr) {
    return std::nullopt;
  }

  state_machine_.stop();

  Instance local_copy = std::move(selected_instance_.value());
  selected_instance_ = std::nullopt;
  return StadiaConnection(std::move(local_copy), std::move(service_deploy_manager_),
                          std::move(grpc_channel_));
}

void ConnectToStadiaWidget::SetupStateMachine() {
  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

  // PROPERTIES of states
  // STATE s_idle
  s_idle_.assignProperty(ui_->retrieveInstancesWidget, "enabled", true);
  s_idle_.assignProperty(ui_->rememberCheckBox, "enabled", false);
  // STATE s_instances_loading
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "statusMessage",
                                      "Loading instances...");
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "cancelable", false);
  s_instances_loading_.assignProperty(ui_->rememberCheckBox, "enabled", false);
  // STATE s_instance_selected
  s_instance_selected_.assignProperty(ui_->retrieveInstancesWidget, "enabled", true);
  s_instance_selected_.assignProperty(ui_->connectButton, "enabled", true);
  // STATE s_waiting_for_creds
  s_loading_credentials_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_loading_credentials_.assignProperty(ui_->instancesTableOverlay, "statusMessage",
                                        "Loading encryption credentials for instance...");
  s_loading_credentials_.assignProperty(ui_->instancesTableOverlay, "cancelable", true);
  // STATE s_deploying
  s_deploying_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_deploying_.assignProperty(ui_->instancesTableOverlay, "cancelable", true);
  // STATE s_connected
  s_connected_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_connected_.assignProperty(ui_->instancesTableOverlay, "spinning", false);
  s_connected_.assignProperty(ui_->instancesTableOverlay, "cancelable", true);
  s_connected_.assignProperty(ui_->instancesTableOverlay, "buttonMessage", "Disconnect");

  // TRANSITIONS (and entered/exit events)
  // STATE s_idle_
  s_idle_.addTransition(ui_->retrieveInstancesWidget, &RetrieveInstancesWidget::LoadingStarted,
                        &s_instances_loading_);
  s_idle_.addTransition(this, &ConnectToStadiaWidget::InstanceSelected, &s_instance_selected_);

  // STATE s_instances_loading_
  s_instances_loading_.addTransition(ui_->retrieveInstancesWidget,
                                     &RetrieveInstancesWidget::LoadingFailed, &s_idle_);
  s_instances_loading_.addTransition(this, &ConnectToStadiaWidget::ReceivedInstances, &s_idle_);

  // STATE s_instance_selected_
  s_instance_selected_.addTransition(this, &ConnectToStadiaWidget::InstancesLoading,
                                     &s_instances_loading_);
  s_instance_selected_.addTransition(ui_->retrieveInstancesWidget,
                                     &RetrieveInstancesWidget::LoadingStarted,
                                     &s_instances_loading_);
  s_instance_selected_.addTransition(this, &ConnectToStadiaWidget::Connecting,
                                     &s_loading_credentials_);
  QObject::connect(&s_instance_selected_, &QState::entered, this, [this]() {
    if (instance_model_.rowCount() == 0) {
      emit InstancesLoading();
    }
  });

  // STATE s_waiting_for_creds_
  QObject::connect(&s_loading_credentials_, &QState::entered, this,
                   &ConnectToStadiaWidget::LoadCredentials);

  s_loading_credentials_.addTransition(this, &ConnectToStadiaWidget::CredentialsLoaded,
                                       &s_deploying_);
  s_loading_credentials_.addTransition(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                                       &s_instance_selected_);
  s_loading_credentials_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred,
                                       &s_instance_selected_);

  // STATE s_deploying_
  QObject::connect(&s_deploying_, &QState::entered, this,
                   &ConnectToStadiaWidget::DeployOrbitService);

  s_deploying_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred, &s_instance_selected_);
  s_deploying_.addTransition(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                             &s_instance_selected_);
  s_deploying_.addTransition(this, &ConnectToStadiaWidget::Connected, &s_connected_);

  // STATE s_connected_
  QObject::connect(&s_connected_, &QState::entered, this, [this] {
    ui_->instancesTableOverlay->SetStatusMessage(
        QString{"Connected to %1"}.arg(selected_instance_->display_name));
  });
  QObject::connect(&s_connected_, &QState::exited, this, &ConnectToStadiaWidget::Disconnect);

  s_connected_.addTransition(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                             &s_instance_selected_);
  s_connected_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred, &s_instance_selected_);
}

void ConnectToStadiaWidget::LoadCredentials() {
  CHECK(selected_instance_.has_value());

  const std::string instance_id{selected_instance_->id.toStdString()};

  if (instance_credentials_.contains(instance_id)) {
    emit CredentialsLoaded();
    return;
  }

  auto future = ggp_client_->GetSshInfoAsync(selected_instance_.value().id, selected_project_);
  future.Then(main_thread_executor_.get(),
              [this, instance_id](ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result) {
                OnSshInfoLoaded(std::move(ssh_info_result), instance_id);
              });
}

void ConnectToStadiaWidget::DeployOrbitService() {
  CHECK(ssh_connection_artifacts_ != nullptr);
  CHECK(service_deploy_manager_ == nullptr);
  CHECK(selected_instance_.has_value());
  const std::string instance_id = selected_instance_->id.toStdString();
  CHECK(instance_credentials_.contains(instance_id));

  const Credentials& credentials{instance_credentials_.at(instance_id)};

  service_deploy_manager_ = std::make_unique<ServiceDeployManager>(
      ssh_connection_artifacts_->GetDeploymentConfiguration(),
      ssh_connection_artifacts_->GetSshContext(), credentials,
      ssh_connection_artifacts_->GetGrpcPort());

  ScopedConnection label_connection{
      QObject::connect(service_deploy_manager_.get(), &ServiceDeployManager::statusMessage,
                       ui_->instancesTableOverlay, &OverlayWidget::SetStatusMessage)};
  ScopedConnection cancel_connection{
      QObject::connect(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                       service_deploy_manager_.get(), &ServiceDeployManager::Cancel)};

  const auto deployment_result = service_deploy_manager_->Exec(metrics_uploader_);
  if (!deployment_result) {
    Disconnect();
    if (deployment_result.error() == make_error_code(Error::kUserCanceledServiceDeployment)) {
      return;
    }
    emit ErrorOccurred(QString("Orbit was unable to successfully connect to the Instance. The "
                               "error message was: %1")
                           .arg(QString::fromStdString(deployment_result.error().message())));
    return;
  }

  QObject::connect(
      service_deploy_manager_.get(), &ServiceDeployManager::socketErrorOccurred, this,
      [this](std::error_code error) {
        emit ErrorOccurred(QString("The connection to instance %1 failed with error: %2")
                               .arg(selected_instance_->display_name)
                               .arg(QString::fromStdString(error.message())));
      });

  CHECK(grpc_channel_ == nullptr);
  grpc_channel_ = CreateGrpcChannel(deployment_result.value().grpc_port);
  CHECK(grpc_channel_ != nullptr);

  emit Connected();
}

void ConnectToStadiaWidget::Disconnect() {
  grpc_channel_ = nullptr;
  service_deploy_manager_ = nullptr;
  ui_->rememberCheckBox->setChecked(false);

  emit Disconnected();
}

void ConnectToStadiaWidget::OnConnectToStadiaRadioButtonClicked(bool checked) {
  if (checked) {
    emit Activated();
  } else {
    ui_->radioButton->setChecked(true);
  }
}

void ConnectToStadiaWidget::OnErrorOccurred(const QString& message) {
  if (IsActive()) {
    QMessageBox::critical(this, QApplication::applicationName(), message);
  } else {
    ERROR("%s", message.toStdString());
  }
}

void ConnectToStadiaWidget::OnSelectionChanged(const QModelIndex& current) {
  if (!current.isValid()) return;

  selected_instance_ = current.data(Qt::UserRole).value<Instance>();
  UpdateRememberInstance(ui_->rememberCheckBox->isChecked());
  emit InstanceSelected();
}

void ConnectToStadiaWidget::UpdateRememberInstance(bool value) {
  QSettings settings;
  if (value) {
    CHECK(selected_instance_ != std::nullopt);
    settings.setValue(kRememberChosenInstance, selected_instance_->id);
  } else {
    settings.remove(kRememberChosenInstance);
    remembered_instance_id_ = std::nullopt;
  }
}

void ConnectToStadiaWidget::OnInstancesLoaded(QVector<orbit_ggp::Instance> instances) {
  instance_model_.SetInstances(std::move(instances));
  emit ReceivedInstances();

  TrySelectRememberedInstance();
}

void ConnectToStadiaWidget::OnSshInfoLoaded(ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result,
                                            std::string instance_id) {
  if (ssh_info_result.has_error()) {
    std::string error_message =
        absl::StrFormat("Unable to load encryption credentials for instance with id %s: %s",
                        instance_id, ssh_info_result.error().message());
    ERROR("%s", error_message);
    emit ErrorOccurred(QString::fromStdString(error_message));
    return;
  }

  LOG("Received ssh info for instance with id: %s", instance_id);

  orbit_ggp::SshInfo& ssh_info{ssh_info_result.value()};
  instance_credentials_.emplace(instance_id, CredentialsFromSshInfo(ssh_info));

  emit CredentialsLoaded();
}

void ConnectToStadiaWidget::TrySelectRememberedInstance() {
  if (remembered_instance_id_ == std::nullopt) return;

  QModelIndexList matches = instance_proxy_model_.match(
      instance_proxy_model_.index(0, static_cast<int>(orbit_ggp::InstanceItemModel::Columns::kId)),
      Qt::DisplayRole, QVariant::fromValue(remembered_instance_id_.value()));

  if (matches.isEmpty()) return;

  ui_->instancesTableView->selectionModel()->setCurrentIndex(
      matches[0], {QItemSelectionModel::SelectCurrent, QItemSelectionModel::Rows});
  emit Connecting();
  remembered_instance_id_ = std::nullopt;
}

bool ConnectToStadiaWidget::IsActive() const { return ui_->contentFrame->isEnabled(); }

void ConnectToStadiaWidget::Connect() {
  CHECK(selected_instance_.has_value());

  auto account_result = GetAccountSync();
  if (account_result.has_error()) {
    emit ErrorOccurred(QString::fromStdString(account_result.error().message()));
    return;
  }

  if (!absl::StartsWith(account_result.value().email.toStdString(),
                        selected_instance_->owner.toStdString())) {
    OtherUserDialog dialog{selected_instance_->owner, this};
    auto dialog_result = dialog.Exec();
    if (dialog_result.has_error()) return;
  }

  emit Connecting();
}

ErrorMessageOr<Account> ConnectToStadiaWidget::GetAccountSync() {
  if (cached_account_ != std::nullopt) return cached_account_.value();

  // This async call is not doing network calls, only reads local config files, hence it is fast and
  // used in a syncronous manner here. A timeout of 3 is used anyways
  ErrorMessageOr<Account> future_result = ErrorMessage{"Call to \"ggp auth list\" timed out."};
  auto account_future = ggp_client_->GetDefaultAccountAsync().Then(
      main_thread_executor_.get(), [&future_result](ErrorMessageOr<Account> result) -> void {
        future_result = std::move(result);
      });

  main_thread_executor_->WaitFor(account_future, std::chrono::seconds(3));

  OUTCOME_TRY(cached_account_, future_result);

  return cached_account_.value();
}

}  // namespace orbit_session_setup