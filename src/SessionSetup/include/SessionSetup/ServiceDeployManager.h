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
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <variant>

#include "DeploymentConfigurations.h"
#include "OrbitBase/AnyInvocable.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToLocalOperation.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"
#include "QtUtils/SingleThreadExecutor.h"

namespace orbit_session_setup {

class ServiceDeployManager : public QObject {
  Q_OBJECT

 public:
  struct GrpcPort {
    uint16_t grpc_port;
  };

  explicit ServiceDeployManager(const DeploymentConfiguration* deployment_configuration,
                                const orbit_ssh::Context* context,
                                orbit_ssh::Credentials credentials, const GrpcPort& grpc_port,
                                QObject* parent = nullptr);

  ~ServiceDeployManager() override;

  ErrorMessageOr<GrpcPort> Exec();

  // This method copies remote source file to local destination.
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> CopyFileToLocal(
      std::filesystem::path source, std::filesystem::path destination,
      orbit_base::StopToken stop_token);

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

  orbit_qt_utils::SingleThreadExecutor background_executor_{};

  void Shutdown();
  ErrorMessageOr<void> ConnectToServer();
  ErrorMessageOr<bool> CheckIfInstalled();
  ErrorMessageOr<void> CopyOrbitServicePackage();
  ErrorMessageOr<void> CopyOrbitServiceExecutable(
      const BareExecutableAndRootPasswordDeployment& config);
  ErrorMessageOr<void> CopyOrbitApiLibrary(const BareExecutableAndRootPasswordDeployment& config);
  ErrorMessageOr<void> CopyOrbitUserSpaceInstrumentationLibrary(
      const BareExecutableAndRootPasswordDeployment& config);
  ErrorMessageOr<void> InstallOrbitServicePackage();
  ErrorMessageOr<void> StartOrbitService(
      const std::variant<SignedDebianPackageDeployment, BareExecutableAndRootPasswordDeployment>&
          deployment_config);
  ErrorMessageOr<uint16_t> StartTunnel(std::optional<orbit_ssh_qt::Tunnel>* tunnel, uint16_t port);
  ErrorMessageOr<std::unique_ptr<orbit_ssh_qt::SftpChannel>> StartSftpChannel();
  ErrorMessageOr<void> ShutdownSftpOperations();
  ErrorMessageOr<void> ShutdownSftpChannel(orbit_ssh_qt::SftpChannel* sftp_channel);
  ErrorMessageOr<void> ShutdownTunnel(orbit_ssh_qt::Tunnel* tunnel);
  ErrorMessageOr<void> ShutdownTask(orbit_ssh_qt::Task* task);
  ErrorMessageOr<void> ShutdownSession(orbit_ssh_qt::Session* session);
  ErrorMessageOr<void> CopyFileToRemote(
      std::string_view source, std::string_view dest,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode dest_mode);

  // TODO(http://b/209807583): With our current integration of libssh2 we can only ever have one
  // copy operation at the same time, so we have a pointer to an ongoing operation and a queue
  // with waiting operations. Since this all happens in one thread, no synchronization is needed.
  orbit_ssh_qt::SftpCopyToLocalOperation* copy_to_local_operation_ = nullptr;
  std::deque<orbit_base::AnyInvocable<void()>> waiting_copy_operations_;

  void CopyFileToLocalImpl(
      orbit_base::Promise<ErrorMessageOr<orbit_base::CanceledOr<void>>> promise,
      std::filesystem::path source, std::filesystem::path destination,
      orbit_base::StopToken stop_token);
  ErrorMessageOr<GrpcPort> ExecImpl();

  void StartWatchdog();

  void handleSocketError(std::error_code);
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_SERVICE_DEPLOY_MANAGER_H_
