// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToTargetDialog.h"

#include <QApplication>
#include <QMessageBox>
#include <memory>

#include "ClientServices/ProcessClient.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Logging.h"
#include "SessionSetup/ConnectToTargetDialog.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "ui_ConnectToTargetDialog.h"

namespace orbit_session_setup {

ConnectToTargetDialog::ConnectToTargetDialog(
    SshConnectionArtifacts* ssh_connection_artifacts, const QString& instance_id,
    uint32_t process_id, orbit_metrics_uploader::MetricsUploader* metrics_uploader, QWidget* parent)
    : QDialog{parent, Qt::Window},
      ui_(std::make_unique<Ui::ConnectToTargetDialog>()),
      ssh_connection_artifacts_(ssh_connection_artifacts),
      instance_id_(instance_id),
      process_id_(process_id),
      metrics_uploader_(metrics_uploader),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
  CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->instanceIdLabel->setText(instance_id);
  ui_->processIdLabel->setText(QString::number(process_id));
}

ConnectToTargetDialog::~ConnectToTargetDialog() {}

std::optional<TargetConfiguration> ConnectToTargetDialog::Exec() {
  LOG("Trying to establish a connection to specified target %s:%d", instance_id_.toStdString(),
      process_id_);

  auto ggp_client_result = orbit_ggp::CreateClient();
  if (ggp_client_result.has_error()) {
    LogAndDisplayError(QString::fromStdString(ggp_client_result.error().message()));
    return std::nullopt;
  }
  ggp_client_ = std::move(ggp_client_result.value());

  SetStatusMessage("Loading encryption credentials for instance...");

  auto process_future = ggp_client_->GetSshInfoAsync(instance_id_, std::nullopt);
  auto instance_future = ggp_client_->DescribeInstanceAsync(instance_id_);
  auto joined_future = orbit_base::JoinFutures(process_future, instance_future);

  std::optional<TargetConfiguration> target;

  joined_future.Then(main_thread_executor_.get(),
                     [this, &target](MaybeSshAndInstanceData ssh_instance_data) {
                       auto result = OnAsyncDataAvailable(std::move(ssh_instance_data));
                       if (result.has_value()) {
                         target = std::move(result.value());
                         accept();
                       } else {
                         LogAndDisplayError(QString::fromStdString(result.error().message()));
                         reject();
                       }
                     });

  int rc = QDialog::exec();

  if (rc != QDialog::Accepted) {
    return std::nullopt;
  }

  return target;
}

ErrorMessageOr<StadiaTarget> ConnectToTargetDialog::OnAsyncDataAvailable(
    MaybeSshAndInstanceData ssh_instance_data) {
  auto& [ssh_info_result, instance_result] = ssh_instance_data;

  std::string error;
  if (ssh_info_result.has_error()) {
    return ssh_info_result.error();
  } else if (instance_result.has_error()) {
    return instance_result.error();
  }

  ConnectionData connection_data;

  connection_data.service_deploy_manager_ =
      std::make_unique<orbit_session_setup::ServiceDeployManager>(
          ssh_connection_artifacts_->GetDeploymentConfiguration(),
          ssh_connection_artifacts_->GetSshContext(),
          CredentialsFromSshInfo(ssh_info_result.value()),
          ssh_connection_artifacts_->GetGrpcPort());

  OUTCOME_TRY(auto&& grpc_port, DeployOrbitService(connection_data.service_deploy_manager_.get()));
  connection_data.grpc_channel_ = CreateGrpcChannel(grpc_port.grpc_port);

  OUTCOME_TRY(connection_data.process_data_,
              FindSpecifiedProcess(connection_data.grpc_channel_, process_id_));

  return CreateTarget(std::move(connection_data), std::move(instance_result.value()));
}

StadiaTarget ConnectToTargetDialog::CreateTarget(ConnectionData result,
                                                 orbit_ggp::Instance instance) const {
  CHECK(instance.id == instance_id_);
  CHECK(result.grpc_channel_ != nullptr);
  CHECK(result.service_deploy_manager_ != nullptr);
  CHECK(result.process_data_ != nullptr);

  auto process_manager =
      orbit_client_services::ProcessManager::Create(result.grpc_channel_, absl::Milliseconds(1000));
  auto stadia_connection = orbit_session_setup::StadiaConnection(
      std::move(instance), std::move(result.service_deploy_manager_),
      std::move(result.grpc_channel_));
  return orbit_session_setup::StadiaTarget(std::move(stadia_connection), std::move(process_manager),
                                           std::move(result.process_data_));
}

ErrorMessageOr<orbit_session_setup::ServiceDeployManager::GrpcPort>
ConnectToTargetDialog::DeployOrbitService(
    orbit_session_setup::ServiceDeployManager* service_deploy_manager) {
  orbit_ssh_qt::ScopedConnection label_connection{
      QObject::connect(service_deploy_manager, &ServiceDeployManager::statusMessage, this,
                       &ConnectToTargetDialog::SetStatusMessage)};
  orbit_ssh_qt::ScopedConnection cancel_connection{
      QObject::connect(ui_->abortButton, &QPushButton::clicked, service_deploy_manager,
                       &ServiceDeployManager::Cancel)};

  auto deployment_result = service_deploy_manager->Exec();
  if (deployment_result.has_error()) {
    return ErrorMessage{"Error during service deployment"};
  } else {
    return deployment_result.value();
  }
}

ErrorMessageOr<std::unique_ptr<orbit_client_data::ProcessData>>
ConnectToTargetDialog::FindSpecifiedProcess(std::shared_ptr<grpc_impl::Channel> grpc_channel,
                                            uint32_t process_id) {
  CHECK(grpc_channel != nullptr);

  orbit_client_services::ProcessClient client(grpc_channel);
  auto process_list = client.GetProcessList();
  if (!process_list.has_value()) {
    return ErrorMessage("Unknown error: Could not retrieve list of running processes.");
  }

  for (auto& process : process_list.value()) {
    if (process.pid() == process_id) {
      return std::make_unique<orbit_client_data::ProcessData>(process);
    }
  }

  return ErrorMessage(QString("PID %1 was not found in the list of running processes.")
                          .arg(process_id)
                          .toStdString());
}

void ConnectToTargetDialog::SetStatusMessage(const QString& message) {
  ui_->statusLabel->setText(QString("<b>Status:</b> ") + message);
}

void ConnectToTargetDialog::LogAndDisplayError(const QString& message) {
  ERROR("%s", message.toStdString());
  QMessageBox::critical(nullptr, QApplication::applicationName(), message);
}

}  // namespace orbit_session_setup