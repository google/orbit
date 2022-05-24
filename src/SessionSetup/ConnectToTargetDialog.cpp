// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToTargetDialog.h"

#include <absl/flags/flag.h>
#include <qobjectdefs.h>

#include <QApplication>
#include <QMessageBox>
#include <QSizePolicy>
#include <algorithm>
#include <memory>

#include "ClientFlags/ClientFlags.h"
#include "ClientServices/ProcessClient.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/WhenAll.h"
#include "SessionSetup/ConnectToTargetDialog.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "grpcpp/channel.h"
#include "ui_ConnectToTargetDialog.h"

namespace orbit_session_setup {

ConnectToTargetDialog::ConnectToTargetDialog(
    SshConnectionArtifacts* ssh_connection_artifacts, const ConnectionTarget& target,
    orbit_metrics_uploader::MetricsUploader* metrics_uploader, QWidget* parent)
    : QDialog{parent, Qt::Window},
      ui_(std::make_unique<Ui::ConnectToTargetDialog>()),
      ssh_connection_artifacts_(ssh_connection_artifacts),
      target_(target),
      metrics_uploader_(metrics_uploader),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
  ORBIT_CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->instanceIdLabel->setText(target_.instance_name_or_id);
  ui_->processIdLabel->setText(target_.process_name_or_path);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
  setSizeGripEnabled(false);
  setFixedSize(window()->sizeHint());
}

ConnectToTargetDialog::~ConnectToTargetDialog() {}

std::optional<TargetConfiguration> ConnectToTargetDialog::Exec() {
  if (absl::GetFlag(FLAGS_launched_from_vsi)) {
    connection_metric_ = std::make_unique<orbit_metrics_uploader::ScopedMetric>(
        metrics_uploader_, orbit_metrics_uploader::OrbitLogEvent::ORBIT_CONNECT_TO_VSI_TARGET);
  } else {
    connection_metric_ = std::make_unique<orbit_metrics_uploader::ScopedMetric>(
        metrics_uploader_, orbit_metrics_uploader::OrbitLogEvent::ORBIT_CONNECT_TO_CLI_TARGET);
  }

  ORBIT_LOG("Trying to establish a connection to process \"%s\" on instance \"%s\"",
            target_.process_name_or_path.toStdString(), target_.instance_name_or_id.toStdString());

  auto ggp_client_result = orbit_ggp::CreateClient();
  if (ggp_client_result.has_error()) {
    LogAndDisplayError(ggp_client_result.error());
    return std::nullopt;
  }
  ggp_client_ = std::move(ggp_client_result.value());

  SetStatusMessage("Loading encryption credentials for instance...");

  auto process_future = ggp_client_->GetSshInfoAsync(target_.instance_name_or_id, std::nullopt);
  auto instance_future = ggp_client_->DescribeInstanceAsync(target_.instance_name_or_id);
  auto joined_future = orbit_base::WhenAll(process_future, instance_future);

  joined_future.Then(main_thread_executor_.get(),
                     [this](MaybeSshAndInstanceData ssh_instance_data) {
                       auto result = OnAsyncDataAvailable(std::move(ssh_instance_data));
                       if (result.has_error()) {
                         LogAndDisplayError(result.error());
                         reject();
                       }
                     });

  int rc = QDialog::exec();

  if (rc != QDialog::Accepted) {
    if (connection_metric_ != nullptr) {
      connection_metric_->SetStatusCode(orbit_metrics_uploader::OrbitLogEvent::CANCELLED);
      connection_metric_.reset();
    }
    return std::nullopt;
  }

  if (connection_metric_ != nullptr) {
    connection_metric_.reset();
  }

  return std::move(target_configuration_);
}

ErrorMessageOr<void> ConnectToTargetDialog::OnAsyncDataAvailable(
    MaybeSshAndInstanceData ssh_instance_data) {
  OUTCOME_TRY(auto&& ssh_info, std::get<0>(ssh_instance_data));
  OUTCOME_TRY(auto&& instance, std::get<1>(ssh_instance_data));

  auto service_deploy_manager = std::make_unique<orbit_session_setup::ServiceDeployManager>(
      ssh_connection_artifacts_->GetDeploymentConfiguration(),
      ssh_connection_artifacts_->GetSshContext(), CredentialsFromSshInfo(ssh_info),
      ssh_connection_artifacts_->GetGrpcPort());

  OUTCOME_TRY(auto&& grpc_port, DeployOrbitService(service_deploy_manager.get()));

  auto grpc_channel = CreateGrpcChannel(grpc_port.grpc_port);
  stadia_connection_ = orbit_session_setup::StadiaConnection(
      std::move(instance), std::move(service_deploy_manager), std::move(grpc_channel));

  process_manager_ = orbit_client_services::ProcessManager::Create(
      stadia_connection_.value().GetGrpcChannel(), absl::Milliseconds(1000));
  process_manager_->SetProcessListUpdateListener(
      [dialog = QPointer<ConnectToTargetDialog>(this)](
          std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
        if (dialog == nullptr) return;
        QMetaObject::invokeMethod(dialog,
                                  [dialog, process_list = std::move(process_list)]() mutable {
                                    dialog->OnProcessListUpdate(std::move(process_list));
                                  });
      });
  SetStatusMessage("Waiting for process to launch.");

  return outcome::success();
}

void ConnectToTargetDialog::OnProcessListUpdate(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
  std::unique_ptr<orbit_client_data::ProcessData> matching_process =
      TryToFindProcessData(process_list, target_.process_name_or_path.toStdString());

  if (matching_process != nullptr) {
    process_manager_->SetProcessListUpdateListener(nullptr);
    target_configuration_ =
        orbit_session_setup::StadiaTarget(std::move(stadia_connection_.value()),
                                          std::move(process_manager_), std::move(matching_process));
    accept();
  }
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

  return service_deploy_manager->Exec();
}

void ConnectToTargetDialog::SetStatusMessage(const QString& message) {
  ui_->statusLabel->setText(QString("<b>Status:</b> ") + message);
}

void ConnectToTargetDialog::LogAndDisplayError(const ErrorMessage& message) {
  ORBIT_ERROR("%s", message.message());

  QMessageBox::critical(nullptr, QApplication::applicationName(),
                        QString::fromStdString(message.message()));
}

}  // namespace orbit_session_setup