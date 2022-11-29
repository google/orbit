// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_CONNECT_TO_SSH_WIDGET_H_
#define SESSION_SETUP_CONNECT_TO_SSH_WIDGET_H_

#include <QRadioButton>
#include <QWidget>
#include <optional>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Credentials.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DeploymentConfigurations.h"

namespace Ui {
class ConnectToSshWidget;
}

namespace orbit_session_setup {

// ConnectToSshWidget provides a UI and functionality to connect to a remote machine via SSH and
// deploy OrbitService.
//
// * For the SSH connection, the user needs to provide 5 fields: Hostname, Port, User, Path to
// known_hosts file and Path to private key file. This can be done in the UI directly, or the UI
// fields can be prefilled via cmd line flags (See `--ssh_...` in ClientFlags.h).
//
// * For the OrbitService deployment the widget provides 2 UI options: "OrbitService started
// manually" (NoDeployment - default) or "Start OrbitService with sudo"
// (BareExecutableAndRootPasswordDeployment). There is also the hidden "Signed Deployment" option,
// which is only visible when the user started Orbit with `--signed_debian_package_deployment`. The
// UI will also be prefilled when the user chooses their deployment method via flags or environment
// variables (see `FigureOutDeploymentConfiguration` in `DeploymentConfigurations.h`).
//
// * After construction of this class, the method `SetSshConnectionArtifacts` needs to be called.
//
// * To reuse an existing `SshConnection` call `SetConnection`.
class ConnectToSshWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ConnectToSshWidget(QWidget* parent = nullptr);
  ~ConnectToSshWidget() override;

  void SetSshConnectionArtifacts(const SshConnectionArtifacts& connection_artifacts);
  void SetConnection(std::optional<SshConnection> connection_opt);
  [[nodiscard]] SshConnection&& TakeConnection();
  [[nodiscard]] std::optional<orbit_ssh::AddrAndPort> GetTargetAddrAndPort() const;

  [[nodiscard]] QRadioButton* GetRadioButton();

 signals:
  void Connected();
  void Disconnected();
  void ProcessListUpdated(QVector<orbit_grpc_protos::ProcessInfo> process_list);

 private:
  std::unique_ptr<Ui::ConnectToSshWidget> ui_;
  std::optional<SshConnection> ssh_connection_;
  DeploymentConfiguration deployment_configuration_;
  std::optional<SshConnectionArtifacts> ssh_connection_artifacts_;

  void OnConnectClicked();
  void OnDisconnectClicked();
  ErrorMessageOr<orbit_ssh::Credentials> GetCredentialsFromUi();
  void UpdateDeploymentConfigurationFromUi();

  ErrorMessageOr<void> TryConnect();
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_CONNECT_TO_SSH_WIDGET_H_