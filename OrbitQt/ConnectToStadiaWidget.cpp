// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConnectToStadiaWidget.h"

#include <absl/strings/str_format.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractState>
#include <QApplication>
#include <QCheckBox>
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
#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>
#include <utility>

#include "Error.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OverlayWidget.h"
#include "servicedeploymanager.h"

namespace {
const QString kRememberChosenInstance{"RememberChosenInstance"};
}  // namespace

namespace orbit_qt {

using orbit_ggp::Instance;
using orbit_ssh_qt::ScopedConnection;

ConnectToStadiaWidget::ConnectToStadiaWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::ConnectToStadiaWidget>()),
      s_idle_(&state_machine_),
      s_instances_loading_(&state_machine_),
      s_instance_selected_(&state_machine_),
      s_waiting_for_creds_(&state_machine_),
      s_deploying_(&state_machine_),
      s_connected_(&state_machine_) {
  ui_->setupUi(this);

  DetachRadioButton();

  QSettings settings;
  if (settings.contains(kRememberChosenInstance)) {
    remembered_instance_id_ = settings.value(kRememberChosenInstance).toString();
    ui_->rememberCheckBox->setChecked(true);
  }

  ui_->instancesTableView->setModel(&instance_model_);

  QObject::connect(ui_->radioButton, &QRadioButton::clicked, this,
                   &ConnectToStadiaWidget::OnConnectToStadiaRadioButtonClicked);
  QObject::connect(this, &ConnectToStadiaWidget::ErrorOccurred, this,
                   &ConnectToStadiaWidget::OnErrorOccurred);
  QObject::connect(ui_->instancesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, &ConnectToStadiaWidget::OnSelectionChanged);
  QObject::connect(ui_->rememberCheckBox, &QCheckBox::toggled, this,
                   &ConnectToStadiaWidget::OnRememberCheckBoxToggled);
  QObject::connect(ui_->refreshButton, &QPushButton::clicked,
                   [this]() { emit InstanceReloadRequested(); });

  SetupStateMachine();
}

void ConnectToStadiaWidget::SetActive(bool value) {
  ui_->contentFrame->setEnabled(value);
  ui_->radioButton->setChecked(value);
}

void ConnectToStadiaWidget::Start(SshConnectionArtifacts* ssh_connection_artifacts,
                                  std::optional<StadiaConnection> connection) {
  CHECK(ssh_connection_artifacts != nullptr);
  ssh_connection_artifacts_ = ssh_connection_artifacts;

  auto client_result = orbit_ggp::Client::Create(this);
  if (!client_result) {
    ui_->radioButton->setToolTip(QString::fromStdString(client_result.error().message()));
    setEnabled(false);
    return;
  }
  ggp_client_ = client_result.value();

  if (connection.has_value()) {
    selected_instance_ = std::move(connection->instance_);
    service_deploy_manager_ = std::move(connection->service_deploy_manager_);
    grpc_channel_ = std::move(connection->grpc_channel_);
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
}

void ConnectToStadiaWidget::SetupStateMachine() {
  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

  // PROPERTIES of states
  // STATE s_idle
  s_idle_.assignProperty(ui_->refreshButton, "enabled", true);
  // STATE s_instances_loading
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "visible", true);
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "statusMessage",
                                      "Loading instances...");
  s_instances_loading_.assignProperty(ui_->instancesTableOverlay, "cancelable", false);
  // STATE s_instance_selected
  s_instance_selected_.assignProperty(ui_->refreshButton, "enabled", true);
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
  s_connected_.assignProperty(ui_->rememberCheckBox, "enabled", true);

  // TRANSITIONS (and entered/exit events)
  // STATE s_idle_
  s_idle_.addTransition(ui_->refreshButton, &QPushButton::clicked, &s_instances_loading_);
  s_idle_.addTransition(this, &ConnectToStadiaWidget::InstanceSelected, &s_instance_selected_);

  // STATE s_instances_loading_
  QObject::connect(&s_instances_loading_, &QState::entered, this,
                   &ConnectToStadiaWidget::ReloadInstances);

  s_instances_loading_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred, &s_idle_);
  s_instances_loading_.addTransition(this, &ConnectToStadiaWidget::ReceivedInstances, &s_idle_);

  // STATE s_instance_selected_
  s_instance_selected_.addTransition(this, &ConnectToStadiaWidget::InstanceReloadRequested,
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
                   &ConnectToStadiaWidget::CheckCredentialsAvailable);

  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ReceivedSshInfo,
                                     &s_waiting_for_creds_);
  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ReadyToDeploy, &s_deploying_);
  s_waiting_for_creds_.addTransition(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                                     &s_instance_selected_);
  s_waiting_for_creds_.addTransition(this, &ConnectToStadiaWidget::ErrorOccurred,
                                     &s_instances_loading_);

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

  ggp_client_->GetInstancesAsync([this](outcome::result<QVector<Instance>> instances) {
    OnInstancesLoaded(std::move(instances));
  });
}

void ConnectToStadiaWidget::CheckCredentialsAvailable() {
  CHECK(selected_instance_.has_value());

  const std::string instance_id = selected_instance_->id.toStdString();

  if (!instance_credentials_.contains(instance_id)) return;

  if (instance_credentials_.at(instance_id).has_error()) {
    emit ErrorOccurred(
        QString::fromStdString(instance_credentials_.at(instance_id).error().message()));
    return;
  }

  emit ReadyToDeploy();
}

void ConnectToStadiaWidget::DeployOrbitService() {
  CHECK(service_deploy_manager_ == nullptr);
  CHECK(selected_instance_.has_value());
  const std::string instance_id = selected_instance_->id.toStdString();
  CHECK(instance_credentials_.contains(instance_id));
  CHECK(instance_credentials_.at(instance_id).has_value());

  const orbit_ssh::Credentials& credentials{instance_credentials_.at(instance_id).value()};

  CHECK(ssh_connection_artifacts_ != nullptr);
  service_deploy_manager_ = std::make_unique<ServiceDeployManager>(
      ssh_connection_artifacts_->GetDeploymentConfiguration(),
      ssh_connection_artifacts_->GetSshContext(), credentials,
      ssh_connection_artifacts_->GetGrpcPort());

  ScopedConnection label_connection{QObject::connect(
      service_deploy_manager_.get(), &orbit_qt::ServiceDeployManager::statusMessage,
      ui_->instancesTableOverlay, &OverlayWidget::SetStatusMessage)};
  ScopedConnection cancel_connection{
      QObject::connect(ui_->instancesTableOverlay, &OverlayWidget::Cancelled,
                       service_deploy_manager_.get(), &orbit_qt::ServiceDeployManager::Cancel)};

  const auto deployment_result = service_deploy_manager_->Exec();
  if (!deployment_result) {
    Disconnect();
    if (deployment_result.error() ==
        make_error_code(orbit_qt::Error::kUserCanceledServiceDeployment)) {
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

  LOG("Deployment successful, grpc_port: %d", deployment_result.value().grpc_port);
  CHECK(grpc_channel_ == nullptr);
  std::string grpc_server_address =
      absl::StrFormat("127.0.0.1:%d", deployment_result.value().grpc_port);
  LOG("Starting gRPC channel to: %s", grpc_server_address);
  grpc_channel_ = grpc::CreateCustomChannel(grpc_server_address, grpc::InsecureChannelCredentials(),
                                            grpc::ChannelArguments());
  CHECK(grpc_channel_ != nullptr);

  emit Connected();
}

void ConnectToStadiaWidget::Disconnect() {
  grpc_channel_ = nullptr;

  // TODO(174561221) currently does not work
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
  if (isVisible()) {
    QMessageBox::critical(this, QApplication::applicationName(), message);
  } else {
    ERROR("%s", message.toStdString());
  }
}

void ConnectToStadiaWidget::OnSelectionChanged(const QModelIndex& current) {
  if (!current.isValid()) return;

  CHECK(current.model() == &instance_model_);
  selected_instance_ = current.data(Qt::UserRole).value<Instance>();
  emit InstanceSelected();
}

void ConnectToStadiaWidget::OnRememberCheckBoxToggled(bool checked) {
  QSettings settings;
  if (checked) {
    CHECK(selected_instance_ != std::nullopt);
    settings.setValue(kRememberChosenInstance, selected_instance_->id);
  } else {
    settings.remove(kRememberChosenInstance);
    remembered_instance_id_ = std::nullopt;
  }
}

void ConnectToStadiaWidget::OnInstancesLoaded(
    outcome::result<QVector<orbit_ggp::Instance>> instances) {
  if (!instances) {
    emit ErrorOccurred(QString("Orbit was unable to retrieve the list of available Stadia "
                               "instances. The error message was: %1")
                           .arg(QString::fromStdString(instances.error().message())));
    return;
  }

  instance_model_.SetInstances(instances.value());
  emit ReceivedInstances();

  TrySelectRememberedInstance();

  for (const auto& instance : instances.value()) {
    std::string instance_id = instance.id.toStdString();

    if (instance_credentials_.contains(instance_id) &&
        instance_credentials_.at(instance_id).has_value()) {
      continue;
    }

    ggp_client_->GetSshInfoAsync(instance,
                                 [this, instance_id = std::move(instance_id)](
                                     outcome::result<orbit_ggp::SshInfo> ssh_info_result) {
                                   OnSshInfoLoaded(std::move(ssh_info_result), instance_id);
                                 });
  }
}

void ConnectToStadiaWidget::OnSshInfoLoaded(outcome::result<orbit_ggp::SshInfo> ssh_info_result,
                                            std::string instance_id) {
  if (!ssh_info_result) {
    std::string error_message =
        absl::StrFormat("Unable to load encryption credentials for instance with id %s: %s",
                        instance_id, ssh_info_result.error().message());
    ERROR("%s", error_message);
    instance_credentials_.emplace(instance_id, ErrorMessage(error_message));
  } else {
    LOG("Received ssh info for instance with id: %s", instance_id);

    orbit_ggp::SshInfo& ssh_info{ssh_info_result.value()};
    orbit_ssh::Credentials credentials;
    credentials.addr_and_port = {ssh_info.host.toStdString(), ssh_info.port};
    credentials.key_path = ssh_info.key_path.toStdString();
    credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
    credentials.user = ssh_info.user.toStdString();

    instance_credentials_.emplace(instance_id, std::move(credentials));
  }

  emit ReceivedSshInfo();
}

void ConnectToStadiaWidget::TrySelectRememberedInstance() {
  if (remembered_instance_id_ == std::nullopt) return;

  QModelIndexList matches = instance_model_.match(
      instance_model_.index(0, static_cast<int>(orbit_ggp::InstanceItemModel::Columns::kId)),
      Qt::DisplayRole, QVariant::fromValue(remembered_instance_id_.value()));

  if (!matches.isEmpty()) {
    ui_->instancesTableView->selectionModel()->setCurrentIndex(
        matches[0], {QItemSelectionModel::SelectCurrent, QItemSelectionModel::Rows});
    emit Connect();
    remembered_instance_id_ = std::nullopt;
  } else {
    ui_->rememberCheckBox->setChecked(false);
  }
}

}  // namespace orbit_qt