// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToTargetDialog.h"

#include <QApplication>
#include <QMessageBox>
#include <memory>

#include "ClientServices/ProcessClient.h"
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
  ui_->processIdLabel->setText(QString("%1").arg(process_id));
}

ConnectToTargetDialog::~ConnectToTargetDialog() {}

std::optional<TargetConfiguration> ConnectToTargetDialog::Exec() {
  int rc = QDialog::DialogCode::Rejected;

  if (StartConnection()) {
    LoadInstanceDetailsAsync();
    rc = QDialog::exec();
  }

  if (rc != QDialog::Accepted) {
    if (!result_data_.error_message_.isEmpty()) {
      QString error =
          QString("Error connecting to specified target: %1.").arg(result_data_.error_message_);
      ERROR("%s", error.toStdString().c_str());
      QMessageBox::critical(nullptr, QApplication::applicationName(), error);
    }
    return std::nullopt;
  }

  return CreateTargetFromResult();
}

StadiaTarget ConnectToTargetDialog::CreateTargetFromResult() {
  CHECK(result_data_.maybe_instance_.has_value());
  CHECK(result_data_.grpc_channel_ != nullptr);
  CHECK(result_data_.service_deploy_manager_ != nullptr);
  CHECK(result_data_.process_data_ != nullptr);

  auto process_manager = orbit_client_services::ProcessManager::Create(result_data_.grpc_channel_,
                                                                       absl::Milliseconds(1000));
  auto stadia_connection = orbit_session_setup::StadiaConnection(
      std::move(result_data_.maybe_instance_.value()),
      std::move(result_data_.service_deploy_manager_), std::move(result_data_.grpc_channel_));
  return orbit_session_setup::StadiaTarget(std::move(stadia_connection), std::move(process_manager),
                                           std::move(result_data_.process_data_));
}

bool ConnectToTargetDialog::StartConnection() {
  LOG("Establishing connection to specified target %s:%d", instance_id_.toStdString().c_str(),
      process_id_);
  result_data_ = ResultData();

  auto ggp_client_result = orbit_ggp::CreateClient();
  if (ggp_client_result.has_error()) {
    result_data_.error_message_ = QString::fromStdString(ggp_client_result.error().message());
    return false;
  }

  ggp_client_ = std::move(ggp_client_result.value());

  SetStatusMessage("Loading encryption credentials for instance...");
  auto future = ggp_client_->GetSshInfoAsync(instance_id_, std::nullopt);
  future.Then(main_thread_executor_.get(),
              [this](ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result) {
                OnSshInfoAvailable(std::move(ssh_info_result));
              });

  return true;
}

void ConnectToTargetDialog::OnSshInfoAvailable(ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result) {
  if (!ssh_info_result.has_error()) {
    LOG("Received ssh info for instance with id: %s", instance_id_.toStdString());

    auto ssh_info = ssh_info_result.value();
    auto deployment_result = DeployOrbitService(ssh_info);
    if (deployment_result.has_value()) {
      result_data_.grpc_channel_ = CreateGrpcChannel(deployment_result.value().grpc_port);
      ListProcessesAndSaveProcess(result_data_.grpc_channel_);
    }
  } else {
    result_data_.error_message_ =
        QString("Error receiving ssh info for instance with id %1:%2")
            .arg(instance_id_, QString::fromStdString(ssh_info_result.error().message()));
  }

  CloseDialogIfResultIsReady();
}

void ConnectToTargetDialog::LoadInstanceDetailsAsync() {
  ggp_client_->DescribeInstanceAsync(instance_id_)
      .Then(main_thread_executor_.get(),
            [this](ErrorMessageOr<orbit_ggp::Instance> instance_result) {
              if (instance_result.has_error()) {
                result_data_.maybe_instance_ = std::nullopt;
                result_data_.error_message_ =
                    QString::fromStdString(instance_result.error().message());
              } else {
                result_data_.maybe_instance_ = instance_result.value();
              }
              CloseDialogIfResultIsReady();
            });
}

std::optional<ServiceDeployManager::GrpcPort> ConnectToTargetDialog::DeployOrbitService(
    orbit_ggp::SshInfo& ssh_info) {
  result_data_.service_deploy_manager_ =
      std::make_unique<orbit_session_setup::ServiceDeployManager>(
          ssh_connection_artifacts_->GetDeploymentConfiguration(),
          ssh_connection_artifacts_->GetSshContext(), CredentialsFromSshInfo(ssh_info),
          ssh_connection_artifacts_->GetGrpcPort());

  orbit_ssh_qt::ScopedConnection label_connection{QObject::connect(
      result_data_.service_deploy_manager_.get(), &ServiceDeployManager::statusMessage, this,
      &ConnectToTargetDialog::SetStatusMessage)};
  orbit_ssh_qt::ScopedConnection cancel_connection{
      QObject::connect(ui_->abortButton, &QPushButton::clicked,
                       result_data_.service_deploy_manager_.get(), &ServiceDeployManager::Cancel)};

  auto deployment_result = result_data_.service_deploy_manager_->Exec();
  if (deployment_result.has_error()) {
    result_data_.error_message_ = QString::fromStdString(deployment_result.error().message());
    return std::nullopt;
  } else {
    return deployment_result.value();
  }
}

void ConnectToTargetDialog::ListProcessesAndSaveProcess(
    std::shared_ptr<grpc_impl::Channel> grpc_channel) {
  CHECK(grpc_channel != nullptr);

  orbit_client_services::ProcessClient client(grpc_channel);
  auto process_list = client.GetProcessList();
  if (!process_list.has_value()) return;

  for (auto& process : process_list.value()) {
    if (process.pid() == process_id_) {
      result_data_.process_data_ = std::make_unique<orbit_client_data::ProcessData>(process);
      return;
    }
  }

  result_data_.error_message_ =
      QString("PID %1 was not found in the list of running processes.").arg(process_id_);
}

void ConnectToTargetDialog::CloseDialogIfResultIsReady() {
  if (!result_data_.error_message_.isEmpty()) {
    reject();
  }

  if (result_data_.maybe_instance_.has_value() && result_data_.grpc_channel_ != nullptr &&
      result_data_.process_data_ != nullptr) {
    accept();
  }
}

void ConnectToTargetDialog::SetStatusMessage(const QString& message) {
  ui_->statusLabel->setText(QString("<b>Status:</b> ") + message);
}

}  // namespace orbit_session_setup