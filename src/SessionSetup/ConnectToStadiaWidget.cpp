// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToStadiaWidget.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
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

#include "ClientFlags/ClientFlags.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitGgp/Project.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/Error.h"
#include "SessionSetup/OverlayWidget.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "ui_ConnectToStadiaWidget.h"

namespace {
const QString kRememberChosenInstance{"RememberChosenInstance"};
const QString kSelectedProjectDisplayNameKey{"kSelectedProjectDisplayNameKey"};
const QString kSelectedProjectIdKey{"kSelectedProjectIdKey"};
const QString kAllInstancesKey{"kAllInstancesKey"};
}  // namespace

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Instance;
using orbit_ggp::Project;
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
      s_waiting_for_creds_(&state_machine_),
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
  QObject::connect(ui_->refreshButton, &QPushButton::clicked, this,
                   [this]() { emit InstanceReloadRequested(); });
  QObject::connect(ui_->instancesFilterLineEdit, &QLineEdit::textChanged, &instance_proxy_model_,
                   &QSortFilterProxyModel::setFilterFixedString);
  QObject::connect(ui_->refreshButton_2, &QPushButton::clicked, this,
                   [this]() { emit InstanceReloadRequested(); });
  QObject::connect(ui_->comboBox, QOverload<int>::of(&QComboBox::activated), this,
                   &ConnectToStadiaWidget::ProjectComboBoxActivated);
  QObject::connect(ui_->allInstancesCheckBox, &QCheckBox::stateChanged, this, [this]() {
    QSettings settings;
    settings.setValue(kAllInstancesKey, ui_->allInstancesCheckBox->isChecked());
  });

  SetupStateMachine();

  SetupProjectSelectionFlagContent();
}

void ConnectToStadiaWidget::SetupProjectSelectionFlagContent() {
  // The instance settings (mainly project selection and "all instances") is currently hidden behind
  // the "--enable_project_selection" flag.
  // TODO(b/190670843): Clean this up when removing the "--enable_project_selection" flag.
  if (absl::GetFlag(FLAGS_enable_project_selection)) {
    // While the "--enable_project_selection" is used, there are 2 refresh buttons in the .ui file.
    // "refreshButton" is used for the old ui, "refreshButton_2" is used for the new ui.
    ui_->refreshButton->hide();
    ui_->comboBox->addItem("Default Project", QVariant());

    QSettings settings;
    QVariant saved_project_id = settings.value(kSelectedProjectIdKey);
    if (saved_project_id.isValid()) {
      // This if branch and the following statements are here to display the previously saved (in
      // QSettings) project while the list of projects is still loading.
      const Project project{settings.value(kSelectedProjectDisplayNameKey).toString(),
                            saved_project_id.toString()};
      LOG("Found previously selected project. display name: %s, id: %s",
          project.display_name.toStdString(), project.id.toStdString());
      SetProject(project);
      ui_->comboBox->addItem(project.display_name, project.id);
      ui_->comboBox->setCurrentIndex(ui_->comboBox->count() - 1);
    }

    if (settings.contains(kAllInstancesKey)) {
      ui_->allInstancesCheckBox->setChecked(settings.value(kAllInstancesKey).toBool());
    }

  } else {
    // refreshButton_2 is part of instancesSettingsWidget and therefore also hidden.
    ui_->instancesSettingsWidget->hide();
  }
}

void ConnectToStadiaWidget::ProjectComboBoxActivated(int index) {
  QVariant selected_id_variant = ui_->comboBox->itemData(index);

  if (!selected_id_variant.isValid()) {  // If "Default Project" is selected
    SetProject(std::nullopt);
    return;
  }

  CHECK(selected_id_variant.canConvert<QString>());
  const QString selected_id = selected_id_variant.toString();

  const auto& result_it =
      std::find_if(projects_.begin(), projects_.end(),
                   [&selected_id](const Project& element) { return element.id == selected_id; });

  CHECK(result_it != projects_.end());

  SetProject(*result_it);
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
  if (ssh_connection_artifacts_ == nullptr) {
    ERROR("Unable to start ConnectToStadiaWidget: ssh_connection_artifacts_ is nullptr");
    return;
  }

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

void ConnectToStadiaWidget::DetachRadioButton() {
  ui_->titleBarLayout->removeWidget(ui_->radioButton);
  ui_->radioButton->setParent(ui_->mainFrame);
  int left = 0;
  int top = 0;
  ui_->mainFrame->layout()->getContentsMargins(&left, &top, nullptr, nullptr);
  int frame_border_width = ui_->mainFrame->lineWidth();
  ui_->radioButton->move(left + frame_border_width, top + frame_border_width);
  ui_->radioButton->show();
}

void ConnectToStadiaWidget::SetupStateMachine() {
  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

  // PROPERTIES of states
  // STATE s_idle
  s_idle_.assignProperty(ui_->refreshButton, "enabled", true);
  s_idle_.assignProperty(ui_->instancesSettingsWidget, "enabled", true);
  s_idle_.assignProperty(ui_->rememberCheckBox, "enabled", false);
  // STATE s_instances_loading
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "statusMessage",
                                      "Loading instances...");
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "cancelable", false);
  s_instances_loading_.assignProperty(ui_->rememberCheckBox, "enabled", false);
  // STATE s_instance_selected
  s_instance_selected_.assignProperty(ui_->refreshButton, "enabled", true);
  s_instance_selected_.assignProperty(ui_->instancesSettingsWidget, "enabled", true);
  s_instance_selected_.assignProperty(ui_->connectButton, "enabled", true);
  // STATE s_waiting_for_creds
  s_waiting_for_creds_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_waiting_for_creds_.assignProperty(ui_->instancesTableOverlay, "statusMessage",
                                      "Loading encryption credentials for instance...");
  s_waiting_for_creds_.assignProperty(ui_->instancesTableOverlay, "cancelable", true);
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
  s_idle_.addTransition(ui_->refreshButton, &QPushButton::clicked, &s_instances_loading_);
  s_idle_.addTransition(ui_->refreshButton_2, &QPushButton::clicked, &s_instances_loading_);
  s_idle_.addTransition(this, &ConnectToStadiaWidget::InstanceReloadRequested,
                        &s_instances_loading_);
  s_idle_.addTransition(ui_->allInstancesCheckBox, &QCheckBox::stateChanged, &s_instances_loading_);
  s_idle_.addTransition(this, &ConnectToStadiaWidget::InstanceSelected, &s_instance_selected_);

  // STATE s_instances_loading_
  QObject::connect(&s_instances_loading_, &QState::entered, this,
                   &ConnectToStadiaWidget::ReloadInstances);

  s_instances_loading_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred, &s_idle_);
  s_instances_loading_.addTransition(this, &ConnectToStadiaWidget::ReceivedInstances, &s_idle_);

  // STATE s_instance_selected_
  s_instance_selected_.addTransition(this, &ConnectToStadiaWidget::InstanceReloadRequested,
                                     &s_instances_loading_);
  s_instance_selected_.addTransition(ui_->allInstancesCheckBox, &QCheckBox::stateChanged,
                                     &s_instances_loading_);
  s_instance_selected_.addTransition(ui_->connectButton, &QPushButton::clicked,
                                     &s_waiting_for_creds_);
  s_instance_selected_.addTransition(ui_->instancesTableView, &QTableView::doubleClicked,
                                     &s_waiting_for_creds_);
  s_instance_selected_.addTransition(this, &ConnectToStadiaWidget::Connect, &s_waiting_for_creds_);
  QObject::connect(&s_instance_selected_, &QState::entered, this, [this]() {
    if (instance_model_.rowCount() == 0) {
      emit InstanceReloadRequested();
    }
  });

  // STATE s_waiting_for_creds_
  QObject::connect(&s_waiting_for_creds_, &QState::entered, this,
                   &ConnectToStadiaWidget::LoadCredentials);

  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ReceivedSshInfo,
                                     &s_waiting_for_creds_);
  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ReadyToDeploy, &s_deploying_);
  s_waiting_for_creds_.addTransition(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                                     &s_instance_selected_);
  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred,
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

void ConnectToStadiaWidget::ReloadInstances() {
  CHECK(ggp_client_ != nullptr);
  instance_model_.SetInstances({});

  orbit_ggp::Client::InstanceListScope scope =
      ui_->allInstancesCheckBox->isChecked()
          ? orbit_ggp::Client::InstanceListScope::kAllReservedInstances
          : orbit_ggp::Client::InstanceListScope::kOnlyOwnInstances;

  ggp_client_->GetInstancesAsync(scope, selected_project_)
      .Then(main_thread_executor_.get(), [this](ErrorMessageOr<QVector<Instance>> instances) {
        OnInstancesLoaded(std::move(instances));
      });

  ggp_client_->GetProjectsAsync().Then(
      main_thread_executor_.get(),
      [this](ErrorMessageOr<QVector<Project>> projects) { OnProjectsLoaded(std::move(projects)); });
}

void ConnectToStadiaWidget::LoadCredentials() {
  CHECK(selected_instance_.has_value());

  const std::string instance_id = selected_instance_->id.toStdString();

  if (instance_credentials_.contains(instance_id)) {
    emit ReadyToDeploy();
    return;
  }

  if (instance_credentials_loading_.contains(instance_id)) return;

  instance_credentials_loading_.emplace(instance_id);
  auto future = ggp_client_->GetSshInfoAsync(selected_instance_.value().id, selected_project_);
  future.Then(main_thread_executor_.get(),
              [this, instance_id](ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result) {
                OnSshInfoLoaded(std::move(ssh_info_result), instance_id);
              });
}

void ConnectToStadiaWidget::DeployOrbitService() {
  CHECK(service_deploy_manager_ == nullptr);
  CHECK(selected_instance_.has_value());
  const std::string instance_id = selected_instance_->id.toStdString();
  CHECK(instance_credentials_.contains(instance_id));

  const orbit_ssh::Credentials& credentials{instance_credentials_.at(instance_id)};

  CHECK(ssh_connection_artifacts_ != nullptr);
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

  // TODO(b/174561221) currently does not work
  // if (service_deploy_manager_ != nullptr) {
  //   service_deploy_manager_->Shutdown();
  // }
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

void ConnectToStadiaWidget::OnInstancesLoaded(
    ErrorMessageOr<QVector<orbit_ggp::Instance>> instances) {
  if (instances.has_error()) {
    emit ErrorOccurred(QString("Orbit was unable to retrieve the list of available Stadia "
                               "instances. The error message was: %1")
                           .arg(QString::fromStdString(instances.error().message())));
    return;
  }

  instance_model_.SetInstances(instances.value());
  emit ReceivedInstances();

  TrySelectRememberedInstance();
}

void ConnectToStadiaWidget::OnProjectsLoaded(ErrorMessageOr<QVector<Project>> projects) {
  if (projects.has_error()) {
    emit ErrorOccurred(
        QString(
            "Orbit was unable to retrieve the list of Stadia projects. The error message was: %1")
            .arg(QString::fromStdString(projects.error().message())));
    return;
  }

  projects_ = std::move(projects.value());
  std::sort(projects_.begin(), projects_.end(), [](const Project& p1, const Project& p2) {
    return p1.display_name.toLower() < p2.display_name.toLower();
  });

  ui_->comboBox->clear();
  ui_->comboBox->addItem("Default Project", QVariant());

  for (const auto& project : projects_) {
    ui_->comboBox->addItem(project.display_name, project.id);
    if (selected_project_ != std::nullopt && selected_project_->id == project.id) {
      ui_->comboBox->setCurrentIndex(ui_->comboBox->count() - 1);  // last added item
    }
  }
}

void ConnectToStadiaWidget::OnSshInfoLoaded(ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result,
                                            std::string instance_id) {
  instance_credentials_loading_.erase(instance_id);

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

  emit ReceivedSshInfo();
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

void ConnectToStadiaWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  // It is important that the call to DetachRadioButton is done here and not during construction.
  // For high dpi display settings in Windows (scaling) the the actual width and height of the radio
  // button is not known during construction. Hence the call is done when the widget is shown, not
  // when its constructed
  DetachRadioButton();
}

bool ConnectToStadiaWidget::IsActive() const { return ui_->contentFrame->isEnabled(); }

void ConnectToStadiaWidget::SetProject(const std::optional<orbit_ggp::Project>& project) {
  if (selected_project_ == project) return;

  QSettings settings;
  if (!project.has_value()) {
    settings.setValue(kSelectedProjectDisplayNameKey, QVariant());
    settings.setValue(kSelectedProjectIdKey, QVariant());
  } else {
    settings.setValue(kSelectedProjectDisplayNameKey, project.value().display_name);
    settings.setValue(kSelectedProjectIdKey, project.value().id);
  }

  selected_project_ = project;
  emit InstanceReloadRequested();
}

}  // namespace orbit_session_setup