// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToSshWidget.h"

#include <absl/flags/flag.h>
#include <absl/flags/internal/flag.h>
#include <qbuttongroup.h>
#include <qcoreapplication.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include <QLineEdit>
#include <QRadioButton>
#include <QValidator>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/OverlayWidget.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupUtils.h"
#include "ui_ConnectToSshWidget.h"

namespace orbit_session_setup {

// The destructor needs to be defined here because it needs to see the type
// `Ui::ConnectToSshWidget`. The header file only contains a forward declaration.
ConnectToSshWidget::~ConnectToSshWidget() = default;

ConnectToSshWidget::ConnectToSshWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ConnectToSshWidget>()) {
  ui_->setupUi(this);
  ui_->overlay->raise();

  ui_->signedDeploymentButton->setVisible(false);

  QObject::connect(ui_->radioButton, &QRadioButton::toggled, ui_->contentContainer,
                   &QWidget::setEnabled);

  QObject::connect(ui_->sudoButton, &QRadioButton::toggled, ui_->sudoPassword,
                   &QLineEdit::setEnabled);

  QObject::connect(ui_->connectButton, &QPushButton::clicked, this,
                   &ConnectToSshWidget::OnConnectClicked);

  QButtonGroup* button_group = new QButtonGroup(this);
  button_group->addButton(ui_->noDeploymentButton);
  button_group->addButton(ui_->signedDeploymentButton);
  button_group->addButton(ui_->sudoButton);

  ui_->port->setValidator(new QIntValidator(1, 65535, this));
  ui_->port->setText(QString::number(absl::GetFlag(FLAGS_ssh_port)));

  if (!absl::GetFlag(FLAGS_ssh_hostname).empty()) {
    ui_->hostname->setText(QString::fromStdString(absl::GetFlag(FLAGS_ssh_hostname)));
  }
  if (!absl::GetFlag(FLAGS_ssh_user).empty()) {
    ui_->user->setText(QString::fromStdString(absl::GetFlag(FLAGS_ssh_user)));
  }
  if (!absl::GetFlag(FLAGS_ssh_known_host_path).empty()) {
    ui_->knownHostsPath->setText(QString::fromStdString(absl::GetFlag(FLAGS_ssh_known_host_path)));
  }
  if (!absl::GetFlag(FLAGS_ssh_key_path).empty()) {
    ui_->keyPath->setText(QString::fromStdString(absl::GetFlag(FLAGS_ssh_key_path)));
  }
}

std::optional<orbit_ssh::AddrAndPort> ConnectToSshWidget::GetTargetAddrAndPort() const {
  if (!ssh_connection_.has_value()) return std::nullopt;

  return ssh_connection_.value().GetAddrAndPort();
}

QRadioButton* ConnectToSshWidget::GetRadioButton() { return ui_->radioButton; }

void ConnectToSshWidget::SetSshConnectionArtifacts(
    const SshConnectionArtifacts& connection_artifacts) {
  // Make a copy of the DeploymentConfiguration and ssh_connection_artifacts_.
  deployment_configuration_ = *connection_artifacts.GetDeploymentConfiguration();
  ssh_connection_artifacts_.emplace(SshConnectionArtifacts{connection_artifacts.GetSshContext(),
                                                           connection_artifacts.GetGrpcPort(),
                                                           &deployment_configuration_});

  if (std::holds_alternative<NoDeployment>(deployment_configuration_)) {
    ui_->noDeploymentButton->setChecked(true);
  }
  if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(deployment_configuration_)) {
    ui_->sudoButton->setChecked(true);
    ui_->sudoPassword->setText(QString::fromStdString(
        std::get<BareExecutableAndRootPasswordDeployment>(deployment_configuration_)
            .root_password));
  }
  if (std::holds_alternative<SignedDebianPackageDeployment>(deployment_configuration_)) {
    ui_->signedDeploymentButton->setVisible(true);
    ui_->signedDeploymentButton->setChecked(true);
  }
}

void ConnectToSshWidget::SetConnection(std::optional<SshConnection> connection_opt) {
  ssh_connection_ = std::move(connection_opt);
  if (!ssh_connection_.has_value()) {
    emit Disconnected();
    ui_->overlay->setVisible(false);
    return;
  }

  ui_->overlay->setVisible(true);
  ui_->overlay->SetSpinning(false);
  ui_->overlay->SetStatusMessage(
      QString("Connected to %1")
          .arg(QString::fromStdString(ssh_connection_->addr_and_port_.GetHumanReadable())));
  ui_->overlay->SetButtonMessage("Disconnect");

  QObject::connect(ui_->overlay, &OverlayWidget::Cancelled, this,
                   &ConnectToSshWidget::OnDisconnectClicked, Qt::UniqueConnection);

  QObject::connect(
      ssh_connection_->GetServiceDeployManager(), &ServiceDeployManager::socketErrorOccurred, this,
      [self = QPointer<ConnectToSshWidget>(this)](std::error_code error) {
        if (self == nullptr) return;

        // Only show a warning message if the widget is enabled.
        if (self->ui_->contentContainer->isEnabled()) {
          QMessageBox::critical(self, "Connection Error",
                                QString("The connection to %1 failed with error message: %2")
                                    .arg(QString::fromStdString(
                                        self->ssh_connection_->GetAddrAndPort().GetHumanReadable()))
                                    .arg(QString::fromStdString(error.message())));
        }
        self->SetConnection(std::nullopt);
      });

  ssh_connection_->GetProcessManager()->SetProcessListUpdateListener(
      [self = QPointer<ConnectToSshWidget>(this)](
          std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
        if (self == nullptr) return;
        emit self->ProcessListUpdated(
            QVector<orbit_grpc_protos::ProcessInfo>(process_list.begin(), process_list.end()));
      });

  emit Connected();
}

[[nodiscard]] SshConnection&& ConnectToSshWidget::TakeConnection() {
  ORBIT_CHECK(ssh_connection_.has_value());
  return std::move(ssh_connection_.value());
}

void ConnectToSshWidget::OnConnectClicked() {
  ErrorMessageOr<void> connected_or_error = TryConnect();

  if (connected_or_error.has_error()) {
    QMessageBox::critical(this, "Error while connecting",
                          QString::fromStdString(connected_or_error.error().message()));
    ui_->overlay->setVisible(false);
  }
}

ErrorMessageOr<orbit_ssh::Credentials> ConnectToSshWidget::GetCredentialsFromUi() {
  bool port_ok = false;
  int port = ui_->port->text().toInt(&port_ok);

  if (ui_->hostname->text().isEmpty() || ui_->user->text().isEmpty() ||
      ui_->knownHostsPath->text().isEmpty() || ui_->keyPath->text().isEmpty() || !port_ok) {
    return ErrorMessage{
        R"(The fields "hostname", "port", "user", "path to known_host file" and "path to private key file" are mandatory)"};
  }

  orbit_ssh::AddrAndPort addr_and_port{ui_->hostname->text().toStdString(), port};
  return orbit_ssh::Credentials{addr_and_port, ui_->user->text().toStdString(),
                                ui_->knownHostsPath->text().toStdString(),
                                ui_->keyPath->text().toStdString()};
}

void ConnectToSshWidget::UpdateDeploymentConfigurationFromUi() {
  if (ui_->sudoButton->isChecked()) {
    const std::filesystem::path orbit_service_path =
        !absl::GetFlag(FLAGS_collector).empty()
            ? std::filesystem::path{absl::GetFlag(FLAGS_collector)}
            : std::filesystem::path{QCoreApplication::applicationDirPath().toStdString()} /
                  "OrbitService";

    deployment_configuration_ = BareExecutableAndRootPasswordDeployment{
        orbit_service_path, ui_->sudoPassword->text().toStdString()};
  } else if (ui_->noDeploymentButton->isChecked()) {
    deployment_configuration_ = NoDeployment{};
  }
}

ErrorMessageOr<void> ConnectToSshWidget::TryConnect() {
  OUTCOME_TRY(orbit_ssh::Credentials credentials, GetCredentialsFromUi());
  UpdateDeploymentConfigurationFromUi();

  auto service_deploy_manager = std::make_unique<orbit_session_setup::ServiceDeployManager>(
      ssh_connection_artifacts_->GetDeploymentConfiguration(),
      ssh_connection_artifacts_->GetSshContext(), credentials,
      ssh_connection_artifacts_->GetGrpcPort());

  ui_->overlay->SetSpinning(true);
  ui_->overlay->SetCancelable(true);
  ui_->overlay->SetStatusMessage(
      QString("Connecting to %1 ...")
          .arg(QString::fromStdString(credentials.addr_and_port.GetHumanReadable())));
  ui_->overlay->SetButtonMessage("Cancel");
  ui_->overlay->setVisible(true);

  orbit_ssh_qt::ScopedConnection cancel_connection{
      QObject::connect(ui_->overlay, &OverlayWidget::Cancelled, service_deploy_manager.get(),
                       &ServiceDeployManager::Cancel)};

  orbit_ssh_qt::ScopedConnection status_message_connection{
      QObject::connect(service_deploy_manager.get(), &ServiceDeployManager::statusMessage,
                       ui_->overlay, &OverlayWidget::SetStatusMessage)};

  OUTCOME_TRY(const ServiceDeployManager::GrpcPort grpc_port, service_deploy_manager->Exec());

  auto grpc_channel = CreateGrpcChannel(grpc_port.grpc_port);

  SetConnection(orbit_session_setup::SshConnection(
      credentials.addr_and_port, std::move(service_deploy_manager), std::move(grpc_channel)));

  return outcome::success();
}

void ConnectToSshWidget::OnDisconnectClicked() {
  ssh_connection_->service_deploy_manager_->Shutdown();
  SetConnection(std::nullopt);
}

}  // namespace orbit_session_setup