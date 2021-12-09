// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_SERVICE_DEPLOY_MANAGER_H_
#define SESSION_SETUP_SERVICE_DEPLOY_MANAGER_H_

#include <stdint.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QThread>
#include <QTimer>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "DeploymentConfigurations.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/AnyInvocable.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"

namespace orbit_session_setup {

class ServiceDeployManager : public QObject {
  Q_OBJECT

 public:
  struct GrpcPort {
    uint16_t grpc_port;
  };

  explicit ServiceDeployManager(const DeploymentConfiguration* deployment_configuration,
                                const orbit_ssh::Context* context, orbit_ssh::Credentials creds,
                                const GrpcPort& grpc_port, QObject* parent = nullptr);

  ~ServiceDeployManager() override;

  outcome::result<GrpcPort> Exec(
      orbit_metrics_uploader::MetricsUploader* metrics_uploader = nullptr);

  // This method copies remote source file to local destination.
  orbit_base::Future<ErrorMessageOr<void>> CopyFileToLocal(std::string source,
                                                           std::string destination);

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
  outcome::result<void> CopyOrbitUserSpaceInstrumentationLibrary(
      const BareExecutableAndRootPasswordDeployment& config);
  outcome::result<void> InstallOrbitServicePackage();
  outcome::result<void> StartOrbitService();
  outcome::result<void> StartOrbitServicePrivileged(
      const BareExecutableAndRootPasswordDeployment& config);
  outcome::result<uint16_t> StartTunnel(std::optional<orbit_ssh_qt::Tunnel>* tunnel, uint16_t port);
  outcome::result<std::unique_ptr<orbit_ssh_qt::SftpChannel>> StartSftpChannel();
  outcome::result<void> ShutdownSftpChannel(orbit_ssh_qt::SftpChannel* sftp_channel);
  outcome::result<void> ShutdownTunnel(orbit_ssh_qt::Tunnel* tunnel);
  outcome::result<void> ShutdownTask(orbit_ssh_qt::Task* task);
  outcome::result<void> ShutdownSession(orbit_ssh_qt::Session* session);
  outcome::result<void> CopyFileToRemote(
      const std::string& source, const std::string& dest,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode dest_mode);

  // TODO(http://b/209807583): With our current integration of libssh2 we can only ever have one
  // copy operation at the same time, so we have a bool indicating an ongoing operation and a queue
  // with waiting operations. Since this all happens in one thread, no synchronization is needed.
  bool copy_file_operation_in_progress_ = false;
  std::deque<orbit_base::AnyInvocable<void()>> waiting_copy_operations_;

  void CopyFileToLocalImpl(orbit_base::Promise<ErrorMessageOr<void>> promise,
                           std::string_view source, std::string_view destination);
  outcome::result<GrpcPort> ExecImpl();

  void StartWatchdog();

  void handleSocketError(std::error_code);
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_SERVICE_DEPLOY_MANAGER_H_
