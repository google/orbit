// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToTargetDialog.h"

#include <absl/time/time.h>

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPointer>
#include <QPushButton>
#include <Qt>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

#include "ClientData/ProcessData.h"
#include "OrbitBase/Logging.h"
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
  ui_->instanceIdLabel->setText(target_.instance_name_or_id);
  ui_->processIdLabel->setText(target_.process_name_or_path);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
  setSizeGripEnabled(false);
  setFixedSize(window()->sizeHint());
}

ConnectToTargetDialog::~ConnectToTargetDialog() {}

std::optional<TargetConfiguration> ConnectToTargetDialog::Exec() {
  ORBIT_LOG("Trying to establish a connection to process \"%s\" on instance \"%s\"",
            target_.process_name_or_path.toStdString(), target_.instance_name_or_id.toStdString());

  ORBIT_FATAL("Not implemented");

  int rc = QDialog::exec();

  if (rc != QDialog::Accepted) {
    return std::nullopt;
  }

  return std::move(target_configuration_);
}

void ConnectToTargetDialog::OnProcessListUpdate(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
  std::unique_ptr<orbit_client_data::ProcessData> matching_process =
      TryToFindProcessData(process_list, target_.process_name_or_path.toStdString());

  if (matching_process != nullptr) {
    process_manager_->SetProcessListUpdateListener(nullptr);
    target_configuration_ =
        orbit_session_setup::SshTarget(std::move(ssh_connection_.value()),
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