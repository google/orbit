// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "servicedeploymanager.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <chrono>
#include <system_error>
#include <thread>

#include "OrbitBase/Logging.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpOperation.h"
#include "OrbitSshQt/Task.h"

static const std::string kLocalhost = "127.0.0.1";
static const std::string kDebDestinationPath = "/tmp/orbitprofiler.deb";
static const std::string kSigDestinationPath = "/tmp/orbitprofiler.deb.asc";

namespace OrbitQt {

ServiceDeployManager::ServiceDeployManager(
    DeploymentConfiguration deployment_configuration,
    OrbitSsh::Credentials credentials, ServiceDeployManager::Ports remote_ports,
    QObject* parent)
    : QObject(parent),
      deployment_configuration_(std::move(deployment_configuration)),
      credentials_(std::move(credentials)),
      remote_ports_(std::move(remote_ports)) {}

void ServiceDeployManager::Cancel() {
  loop_.error(std::make_error_code(std::errc::operation_canceled));
}

outcome::result<bool> ServiceDeployManager::CheckIfInstalled() {
  emit statusMessage(
      QString(
          "Checking if OrbitService is already installed in version %1 on the "
          "remote instance.")
          .arg(QApplication::applicationVersion()));

  auto version = QApplication::applicationVersion().toStdString();
  if (!version.empty() && version.front() == 'v') {
    // The old git tags have a 'v' in front which is not supported by debian
    // packages. So we have to remove it.
    version = version.substr(1);
  }
  const auto command = absl::StrFormat(
      "/usr/bin/dpkg-query -W orbitprofiler 2>/dev/null | grep %s", version);

  OrbitSshQt::Task checkIfInstalledTask{&session_.value(), command,
                                        OrbitSshQt::Task::Tty::kNo};

  QObject::connect(&checkIfInstalledTask, &OrbitSshQt::Task::finished, &loop_,
                   &EventLoop::exit);

  auto error_handler = ConnectErrorHandler(&checkIfInstalledTask,
                                           &OrbitSshQt::Task::errorOccurred);

  checkIfInstalledTask.Start();

  OUTCOME_TRY(result, loop_.exec());
  if (result == 0) {
    // Already installed
    emit statusMessage(
        "The correct version of OrbitService is already installed.");
    return outcome::success(true);
  } else {
    emit statusMessage(
        "The correct version of OrbitService is not yet installed.");
    return outcome::success(false);
  }
}

outcome::result<uint16_t> ServiceDeployManager::StartTunnel(
    std::optional<OrbitSshQt::Tunnel>* tunnel, uint16_t port) {
  emit statusMessage("Setting up port forwarding...");
  LOG("Setting up tunnel on port %d", remote_ports_.grpc_port);

  tunnel->emplace(&session_.value(), kLocalhost, port);

  auto error_handler =
      ConnectErrorHandler(&tunnel->value(), &OrbitSshQt::Tunnel::errorOccurred);
  auto quitHandler =
      ConnectQuitHandler(&tunnel->value(), &OrbitSshQt::Tunnel::started);

  tunnel->value().Start();

  OUTCOME_TRY(loop_.exec());
  QObject::connect(&tunnel->value(), &OrbitSshQt::Tunnel::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success(tunnel->value().GetListenPort());
}

outcome::result<void> ServiceDeployManager::StartSftpChannel(
    OrbitSshQt::SftpChannel* channel) {
  auto quitHandler =
      ConnectQuitHandler(channel, &OrbitSshQt::SftpChannel::started);

  auto error_handler =
      ConnectErrorHandler(channel, &OrbitSshQt::SftpChannel::errorOccurred);

  channel->Start();

  OUTCOME_TRY(loop_.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::CopyFileToRemote(
    OrbitSshQt::SftpChannel& channel, std::string source, std::string dest,
    OrbitSshQt::SftpOperation::FileMode dest_mode) {
  OrbitSshQt::SftpOperation operation{&session_.value(), &channel};

  auto quitHandler =
      ConnectQuitHandler(&operation, &OrbitSshQt::SftpOperation::stopped);

  auto error_handler = ConnectErrorHandler(
      &operation, &OrbitSshQt::SftpOperation::errorOccurred);

  LOG("About to start copying from %s to %s...", source, dest);
  operation.CopyFileToRemote(source, dest, dest_mode);

  OUTCOME_TRY(loop_.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StopSftpChannel(
    OrbitSshQt::SftpChannel* channel) {
  auto quitHandler =
      ConnectQuitHandler(channel, &OrbitSshQt::SftpChannel::stopped);

  auto error_handler =
      ConnectErrorHandler(channel, &OrbitSshQt::SftpChannel::errorOccurred);

  channel->Stop();

  OUTCOME_TRY(loop_.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::CopyOrbitServicePackage() {
  emit statusMessage("Copying OrbitService package to the remote instance...");

  OrbitSshQt::SftpChannel channel{&session_.value()};

  OUTCOME_TRY(StartSftpChannel(&channel));
  LOG("SFTP Channel open.");

  auto& config =
      std::get<SignedDebianPackageDeployment>(deployment_configuration_);

  using FileMode = OrbitSshQt::SftpOperation::FileMode;
  OUTCOME_TRY(CopyFileToRemote(channel, config.path_to_package.string(),
                               kDebDestinationPath, FileMode::kUserWritable));

  OUTCOME_TRY(CopyFileToRemote(channel, config.path_to_signature.string(),
                               kSigDestinationPath, FileMode::kUserWritable));

  OUTCOME_TRY(StopSftpChannel(&channel));

  emit statusMessage(
      "Finished copying the OrbitService package to the remote instance.");
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::CopyOrbitServiceExecutable() {
  emit statusMessage(
      "Copying OrbitService executable to the remote instance...");

  OrbitSshQt::SftpChannel channel{&session_.value()};

  OUTCOME_TRY(StartSftpChannel(&channel));
  LOG("SFTP Channel open.");

  const std::string exe_destination_path = "/tmp/OrbitService";
  auto& config = std::get<BareExecutableAndRootPasswordDeployment>(
      deployment_configuration_);

  OUTCOME_TRY(CopyFileToRemote(
      channel, config.path_to_executable.string(), exe_destination_path,
      OrbitSshQt::SftpOperation::FileMode::kUserWritableAllExecutable));

  OUTCOME_TRY(StopSftpChannel(&channel));

  emit statusMessage(
      "Finished copying the OrbitService executable to the remote instance.");
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StartOrbitService() {
  emit statusMessage("Starting OrbitService on the remote instance...");

  orbit_service_task_.emplace(&session_.value(),
                              "/opt/developer/tools/OrbitService",
                              OrbitSshQt::Task::Tty::kNo);

  auto quitHandler = ConnectQuitHandler(&orbit_service_task_.value(),
                                        &OrbitSshQt::Task::started);

  auto error_handler = ConnectErrorHandler(&orbit_service_task_.value(),
                                           &OrbitSshQt::Task::errorOccurred);

  QObject::connect(
      &orbit_service_task_.value(), &OrbitSshQt::Task::readyRead, this,
      [this]() {
        const auto buf = orbit_service_task_->Read();
        const auto lines = QString::fromStdString(buf).split('\n');
        for (const auto& line : lines) {
          LOG("%s", QString("[OrbitService] %1").arg(line).toStdString());
        }
      });

  orbit_service_task_->Start();

  OUTCOME_TRY(loop_.exec());
  QObject::connect(&orbit_service_task_.value(),
                   &OrbitSshQt::Task::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StartOrbitServicePrivileged() {
  emit statusMessage("Starting OrbitService on the remote instance...");

  orbit_service_task_.emplace(&session_.value(), "sudo /tmp/OrbitService",
                              OrbitSshQt::Task::Tty::kYes);

  const auto& config =
      std::get<OrbitQt::BareExecutableAndRootPasswordDeployment>(
          deployment_configuration_);

  auto quitHandler = OrbitSshQt::ScopedConnection{QObject::connect(
      &orbit_service_task_.value(), &OrbitSshQt::Task::bytesWritten, this,
      [&, bytes_accumulated = 0u](size_t bytesWritten) mutable {
        bytes_accumulated += bytesWritten;
        if (bytes_accumulated >= config.root_password.size() + 1) {
          loop_.quit();

          QObject::connect(&orbit_service_task_.value(),
                           &OrbitSshQt::Task::readyRead, this, [this]() {
                             const auto buf = orbit_service_task_->Read();
                             std::vector<std::string_view> lines =
                                 absl::StrSplit(buf, '\n');
                             for (const auto& line : lines) {
                               LOG("[OrbitService] %s", line);
                             }
                           });
        }
      })};

  auto error_handler = ConnectErrorHandler(&orbit_service_task_.value(),
                                           &OrbitSshQt::Task::errorOccurred);

  auto waitForPasswordRequestHandler =
      OrbitSshQt::ScopedConnection{QObject::connect(
          &orbit_service_task_.value(), &OrbitSshQt::Task::readyRead, this,
          [&, buf = std::string{}]() mutable {
            buf.append(orbit_service_task_->Read());

            if (absl::StrContains(buf, "[sudo] password for cloudcast:")) {
              orbit_service_task_->Write(
                  absl::StrFormat("%s\n", config.root_password));
            }
          })};

  orbit_service_task_->Start();

  OUTCOME_TRY(loop_.exec());
  QObject::connect(&orbit_service_task_.value(),
                   &OrbitSshQt::Task::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::InstallOrbitServicePackage() {
  emit statusMessage(
      "Installing the OrbitService package on the remote instance...");

  const auto command =
      absl::StrFormat("sudo /usr/sbin/install_signed_package.sh %s %s",
                      kDebDestinationPath, kSigDestinationPath);
  OrbitSshQt::Task installServiceTask{&session_.value(), command,
                                      OrbitSshQt::Task::Tty::kNo};

  QObject::connect(
      &installServiceTask, &OrbitSshQt::Task::finished, this,
      [&](int exit_code) {
        if (exit_code == 0) {
          loop_.quit();
        } else {
          // TODO(hebecker): Replace this generic error code with a custom one.
          loop_.error(std::make_error_code(std::errc::permission_denied));
        }
      });

  auto error_handler = ConnectErrorHandler(&installServiceTask,
                                           &OrbitSshQt::Task::errorOccurred);

  installServiceTask.Start();

  OUTCOME_TRY(loop_.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::ConnectToServer() {
  emit statusMessage(QString("Connecting to %1:%2...")
                         .arg(QString::fromStdString(credentials_.host))
                         .arg(credentials_.port));

  OUTCOME_TRY(context, OrbitSsh::Context::Create());
  context_ = std::move(context);

  session_.emplace(&context_.value());

  using OrbitSshQt::Session;
  auto quitHandler = ConnectQuitHandler(&session_.value(), &Session::started);
  auto error_handler =
      ConnectErrorHandler(&session_.value(), &Session::errorOccurred);

  session_->ConnectToServer(credentials_);
  OUTCOME_TRY(loop_.exec());
  emit statusMessage(QString("Successfully connected to %1:%2.")
                         .arg(QString::fromStdString(credentials_.host))
                         .arg(credentials_.port));

  QObject::connect(&session_.value(), &Session::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

outcome::result<ServiceDeployManager::Ports> ServiceDeployManager::Exec() {
  OUTCOME_TRY(ConnectToServer());

  // Release mode: Deploying a signed debian package. No password required.
  if (std::holds_alternative<SignedDebianPackageDeployment>(
          deployment_configuration_)) {
    OUTCOME_TRY(service_already_installed, CheckIfInstalled());

    if (!service_already_installed) {
      OUTCOME_TRY(CopyOrbitServicePackage());
      OUTCOME_TRY(InstallOrbitServicePackage());
    }
    OUTCOME_TRY(StartOrbitService());
    // TODO(hebecker): Replace this timeout by waiting for a
    // stdout-greeting-message.
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    // Developer mode: Deploying a bare executable and start it via sudo.
  } else if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(
                 deployment_configuration_)) {
    OUTCOME_TRY(CopyOrbitServiceExecutable());
    OUTCOME_TRY(StartOrbitServicePrivileged());
    // TODO(hebecker): Replace this timeout by waiting for a
    // stdout-greeting-message.
    std::this_thread::sleep_for(std::chrono::milliseconds{200});

    // Manual Developer mode: No deployment, no starting. Just the tunnels.
  } else if (std::holds_alternative<NoDeployment>(deployment_configuration_)) {
    // Nothing to deploy
    emit statusMessage(
        "Skipping deployment step. Expecting that OrbitService is already "
        "running...");
  }

  OUTCOME_TRY(local_asio_port,
              StartTunnel(&asio_tunnel_, remote_ports_.asio_port));
  OUTCOME_TRY(local_grpc_port,
              StartTunnel(&grpc_tunnel_, remote_ports_.grpc_port));

  emit statusMessage("Successfully set up port forwarding!");

  LOG("Local ports for ASIO and GRPC are %d and %d.", local_asio_port,
      local_grpc_port);
  return outcome::success(Ports{/* .asio_port = */ local_asio_port,
                                /* .grpc_port = */ local_grpc_port});
}

void ServiceDeployManager::handleSocketError(std::error_code e) {
  LOG("Socket error: %s", e.message());
  emit socketErrorOccurred(e);
}

void ServiceDeployManager::ShutdownTunnel(
    std::optional<OrbitSshQt::Tunnel>* tunnel) {
  if (!tunnel || !*tunnel) {
    return;
  }

  auto quitHandler =
      ConnectQuitHandler(&tunnel->value(), &OrbitSshQt::Tunnel::started);
  auto error_handler =
      ConnectQuitHandler(&tunnel->value(), &OrbitSshQt::Tunnel::errorOccurred);

  tunnel->value().Stop();

  (void)loop_.exec();
  *tunnel = std::nullopt;
}

void ServiceDeployManager::ShutdownOrbitService() {
  if (!orbit_service_task_) {
    return;
  }

  auto quitHandler = ConnectQuitHandler(&orbit_service_task_.value(),
                                        &OrbitSshQt::Task::finished);
  auto error_handler = ConnectQuitHandler(&orbit_service_task_.value(),
                                          &OrbitSshQt::Task::errorOccurred);

  orbit_service_task_->Stop();

  (void)loop_.exec();
  orbit_service_task_ = std::nullopt;
}

void ServiceDeployManager::ShutdownSession() {
  if (!session_) {
    return;
  }

  auto quitHandler =
      ConnectQuitHandler(&session_.value(), &OrbitSshQt::Session::stopped);
  auto error_handler = ConnectQuitHandler(&session_.value(),
                                          &OrbitSshQt::Session::errorOccurred);

  session_->Disconnect();

  (void)loop_.exec();
  session_ = std::nullopt;
}

void ServiceDeployManager::Shutdown() {
  ShutdownTunnel(&grpc_tunnel_);
  ShutdownTunnel(&asio_tunnel_);
  ShutdownOrbitService();
  ShutdownSession();
}
}  // namespace OrbitQt
