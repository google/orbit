// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToTargetDialog.h"

#include <absl/strings/str_format.h>

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPointer>
#include <QPushButton>
#include <Qt>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "ClientData/ProcessData.h"
#include "ClientServices/ProcessManager.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "SessionSetup/TargetConfiguration.h"
#include "ui_ConnectToTargetDialog.h"

namespace orbit_session_setup {

ConnectToTargetDialog::ConnectToTargetDialog(SshConnectionArtifacts* ssh_connection_artifacts,
                                             const ConnectionTarget& target, QWidget* parent)
    : QDialog{parent, Qt::Window},
      ui_(std::make_unique<Ui::ConnectToTargetDialog>()),
      ssh_connection_artifacts_(ssh_connection_artifacts),
      target_(target),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
  ORBIT_CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->sshTargetLabel->setText(
      QString::fromStdString(target_.credentials.addr_and_port.GetHumanReadable()));
  ui_->processIdLabel->setText(target_.process_name_or_path);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
  setSizeGripEnabled(false);
  setFixedSize(window()->sizeHint());
}

ConnectToTargetDialog::~ConnectToTargetDialog() {}

std::optional<TargetConfiguration> ConnectToTargetDialog::Exec() {
  const std::string status_message =
      absl::StrFormat(R"(Trying to establish a connection to process "%s" on SSH target "%s")",
                      target_.process_name_or_path.toStdString(),
                      target_.credentials.addr_and_port.GetHumanReadable());
  ORBIT_LOG("%s", status_message);

  SetStatusMessage(QString::fromStdString(status_message));

  // The call to `DeployOrbitServiceAndSetupProcessManager` is scheduled on the
  // main thread, so it happens after the dialog is shown (via call to `Exec`).
  main_thread_executor_->Schedule([this]() {
    ErrorMessageOr<void> deploy_result = DeployOrbitServiceAndSetupProcessManager();
    if (deploy_result.has_error()) {
      LogAndDisplayError(deploy_result.error());
      reject();
    }
  });

  int rc = QDialog::exec();

  if (rc != QDialog::Accepted) {
    return std::nullopt;
  }

  return std::move(target_configuration_);
}

void ConnectToTargetDialog::OnProcessListUpdate(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
  std::unique_ptr<orbit_client_data::ProcessData> matching_process =
      TryToFindProcessData(std::move(process_list), target_.process_name_or_path.toStdString());

  if (matching_process != nullptr) {
    ssh_connection_->GetProcessManager()->SetProcessListUpdateListener(nullptr);
    target_configuration_ = orbit_session_setup::SshTarget(std::move(ssh_connection_.value()),
                                                           std::move(matching_process));
    accept();
  }
}

ErrorMessageOr<void> ConnectToTargetDialog::DeployOrbitServiceAndSetupProcessManager() {
  auto service_deploy_manager = std::make_unique<orbit_session_setup::ServiceDeployManager>(
      ssh_connection_artifacts_->GetDeploymentConfiguration(),
      ssh_connection_artifacts_->GetSshContext(), target_.credentials,
      ssh_connection_artifacts_->GetGrpcPort());

  orbit_ssh_qt::ScopedConnection label_connection{
      QObject::connect(service_deploy_manager.get(), &ServiceDeployManager::statusMessage, this,
                       &ConnectToTargetDialog::SetStatusMessage)};
  orbit_ssh_qt::ScopedConnection cancel_connection{
      QObject::connect(ui_->abortButton, &QPushButton::clicked, service_deploy_manager.get(),
                       &ServiceDeployManager::Cancel)};

  OUTCOME_TRY(const ServiceDeployManager::GrpcPort grpc_port, service_deploy_manager->Exec());

  auto grpc_channel = CreateGrpcChannel(grpc_port.grpc_port);
  ssh_connection_ = orbit_session_setup::SshConnection(target_.credentials.addr_and_port,
                                                       std::move(service_deploy_manager),
                                                       std::move(grpc_channel));

  ssh_connection_->GetProcessManager()->SetProcessListUpdateListener(
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

void ConnectToTargetDialog::SetStatusMessage(const QString& message) {
  ui_->statusLabel->setText(QString("<b>Status:</b> ") + message);
}

void ConnectToTargetDialog::LogAndDisplayError(const ErrorMessage& message) {
  ORBIT_ERROR("%s", message.message());

  QMessageBox::critical(nullptr, QApplication::applicationName(),
                        QString::fromStdString(message.message()));
}

}  // namespace orbit_session_setup