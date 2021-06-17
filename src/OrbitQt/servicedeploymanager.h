// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_
#define ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_

#include <stdint.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QThread>
#include <QTimer>
#include <memory>
#include <optional>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <system_error>

#include "DeploymentConfigurations.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"

namespace orbit_qt {

class ServiceDeployManager : public QObject {
  Q_OBJECT

 public:
  struct GrpcPort {
    uint16_t grpc_port;
  };

  explicit ServiceDeployManager(const DeploymentConfiguration* deployment_configuration,
                                const orbit_ssh::Context* context, orbit_ssh::Credentials creds,
                                const GrpcPort& grpc_port, QObject* parent = nullptr);

  ~ServiceDeployManager() noexcept;

  outcome::result<GrpcPort> Exec();

  // This method copies remote source file to local destination.
  ErrorMessageOr<void> CopyFileToLocal(std::string source, std::string destination);

  void Shutdown();
  void Cancel();

 signals:
  void statusMessage(const QString&);
  void socketErrorOccurred(std::error_code);
  void cancelRequested();

 private:
  const DeploymentConfiguration* deployment_configuration_;
  const orbit_ssh::Context* context_;
  orbit_ssh::Credentials credentials_;
  GrpcPort grpc_port_;
  std::optional<orbit_ssh_qt::Session> session_;
  std::optional<orbit_ssh_qt::Task> orbit_service_task_;
  std::optional<orbit_ssh_qt::Tunnel> grpc_tunnel_;
  std::unique_ptr<orbit_ssh_qt::SftpChannel> sftp_channel_;
  QTimer ssh_watchdog_timer_;

  QThread background_thread_;

  outcome::result<void> ConnectToServer();
  outcome::result<bool> CheckIfInstalled();
  outcome::result<void> CopyOrbitServicePackage();
  outcome::result<void> CopyOrbitServiceExecutable(
      const BareExecutableAndRootPasswordDeployment& config);
  outcome::result<void> CopyOrbitApiLibrary(const BareExecutableAndRootPasswordDeployment& config);
  outcome::result<void> InstallOrbitServicePackage();
  outcome::result<void> StartOrbitService();
  outcome::result<void> StartOrbitServicePrivileged(
      const BareExecutableAndRootPasswordDeployment& config);
  outcome::result<uint16_t> StartTunnel(std::optional<orbit_ssh_qt::Tunnel>* tunnel, uint16_t port);
  outcome::result<std::unique_ptr<orbit_ssh_qt::SftpChannel>> StartSftpChannel();
  outcome::result<void> StopSftpChannel(orbit_ssh_qt::SftpChannel* sftp_channel);
  outcome::result<void> CopyFileToRemote(
      const std::string& source, const std::string& dest,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode dest_mode);

  ErrorMessageOr<void> CopyFileToLocalImpl(std::string_view source, std::string_view destination);
  outcome::result<GrpcPort> ExecImpl();

  void StartWatchdog();
  void StopSftpChannel();
  void ShutdownSession();
  void ShutdownOrbitService();
  void ShutdownTunnel(std::optional<orbit_ssh_qt::Tunnel>*);

  void handleSocketError(std::error_code);
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_
