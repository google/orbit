// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_
#define ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_

#include <QEventLoop>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <string>

#include "OrbitBase/Result.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"
#include "deploymentconfigurations.h"
#include "eventloop.h"

namespace OrbitQt {

class ServiceDeployManager : public QObject {
  Q_OBJECT

 public:
  struct GrpcPort {
    uint16_t grpc_port;
  };

  explicit ServiceDeployManager(
      DeploymentConfiguration deployment_configuration,
      OrbitSsh::Context* context, OrbitSsh::Credentials creds,
      const GrpcPort& grpc_port, QObject* parent = nullptr);

  outcome::result<GrpcPort> Exec();

  // This method copies remote source file to local destination.
  ErrorMessageOr<void> CopyFileToLocal(std::string_view source,
                                       std::string_view destination);

  void Shutdown();
  void Cancel();

 signals:
  void statusMessage(const QString&);
  void socketErrorOccurred(std::error_code);

 private:
  EventLoop loop_;

  DeploymentConfiguration deployment_configuration_;
  OrbitSsh::Context* context_ = nullptr;
  OrbitSsh::Credentials credentials_;
  GrpcPort grpc_port_;
  std::optional<OrbitSshQt::Session> session_;
  std::optional<OrbitSshQt::Task> orbit_service_task_;
  std::optional<OrbitSshQt::Tunnel> grpc_tunnel_;
  std::unique_ptr<OrbitSshQt::SftpChannel> sftp_channel_;
  QTimer ssh_watchdog_timer_;

  outcome::result<void> ConnectToServer();
  outcome::result<bool> CheckIfInstalled();
  outcome::result<void> CopyOrbitServicePackage();
  outcome::result<void> CopyOrbitServiceExecutable();
  outcome::result<void> InstallOrbitServicePackage();
  outcome::result<void> StartOrbitService();
  outcome::result<void> StartOrbitServicePrivileged();
  outcome::result<uint16_t> StartTunnel(
      std::optional<OrbitSshQt::Tunnel>* tunnel, uint16_t port);
  outcome::result<std::unique_ptr<OrbitSshQt::SftpChannel>> StartSftpChannel(
      EventLoop* loop);
  outcome::result<void> StopSftpChannel(EventLoop* loop,
                                        OrbitSshQt::SftpChannel* sftp_channel);
  outcome::result<void> CopyFileToRemote(
      const std::string& source, const std::string& dest,
      OrbitSshQt::SftpCopyToRemoteOperation::FileMode dest_mode);

  void StartWatchdog();
  void StopSftpChannel();
  void ShutdownSession();
  void ShutdownOrbitService();
  void ShutdownTunnel(std::optional<OrbitSshQt::Tunnel>*);

  void handleSocketError(std::error_code);

  template <typename Func>
  [[nodiscard]] OrbitSshQt::ScopedConnection ConnectQuitHandler(
      const typename QtPrivate::FunctionPointer<Func>::Object* sender,
      Func signal) {
    return ConnectQuitHandler(&loop_, sender, signal);
  }

  template <typename Func>
  [[nodiscard]] OrbitSshQt::ScopedConnection ConnectQuitHandler(
      EventLoop* loop,
      const typename QtPrivate::FunctionPointer<Func>::Object* sender,
      Func signal) {
    return OrbitSshQt::ScopedConnection{
        QObject::connect(sender, signal, loop, &EventLoop::quit)};
  }

  template <typename Func>
  [[nodiscard]] OrbitSshQt::ScopedConnection ConnectErrorHandler(
      const typename QtPrivate::FunctionPointer<Func>::Object* sender,
      Func signal) {
    return ConnectErrorHandler(&loop_, sender, signal);
  }

  template <typename Func>
  [[nodiscard]] OrbitSshQt::ScopedConnection ConnectErrorHandler(
      EventLoop* loop,
      const typename QtPrivate::FunctionPointer<Func>::Object* sender,
      Func signal) {
    return OrbitSshQt::ScopedConnection{
        QObject::connect(sender, signal, loop, &EventLoop::error)};
  }
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_SERVICE_DEPLOY_MANAGER_H_