// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/servicedeploymanager.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <QApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <Qt>
#include <chrono>
#include <filesystem>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToLocalOperation.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "QtUtils/EventLoop.h"
#include "SessionSetup/Error.h"

ABSL_DECLARE_FLAG(bool, devmode);

static const std::string kLocalhost = "127.0.0.1";
static const std::string kDebDestinationPath = "/tmp/orbitprofiler.deb";
static const std::string kSigDestinationPath = "/tmp/orbitprofiler.deb.asc";
static const std::string_view kSshWatchdogPassphrase = "start_watchdog";
static const std::chrono::milliseconds kSshWatchdogInterval(1000);

namespace orbit_session_setup {

namespace {
template <typename Func>
[[nodiscard]] orbit_ssh_qt::ScopedConnection ConnectQuitHandler(
    orbit_qt_utils::EventLoop* loop,
    const typename QtPrivate::FunctionPointer<Func>::Object* sender, Func signal) {
  return orbit_ssh_qt::ScopedConnection{
      QObject::connect(sender, signal, loop, &orbit_qt_utils::EventLoop::quit)};
}

template <typename Func>
[[nodiscard]] orbit_ssh_qt::ScopedConnection ConnectErrorHandler(
    orbit_qt_utils::EventLoop* loop,
    const typename QtPrivate::FunctionPointer<Func>::Object* sender, Func signal) {
  return orbit_ssh_qt::ScopedConnection{
      QObject::connect(sender, signal, loop, &orbit_qt_utils::EventLoop::error)};
}

[[nodiscard]] orbit_ssh_qt::ScopedConnection ConnectCancelHandler(orbit_qt_utils::EventLoop* loop,
                                                                  ServiceDeployManager* sdm) {
  return orbit_ssh_qt::ScopedConnection{QObject::connect(
      sdm, &ServiceDeployManager::cancelRequested, loop,
      [loop]() { loop->error(make_error_code(Error::kUserCanceledServiceDeployment)); })};
}

void PrintAsOrbitService(const std::string& buffer) {
  std::vector<std::string_view> lines = absl::StrSplit(buffer, '\n');
  for (const auto& line : lines) {
    if (!line.empty()) {
      PLATFORM_LOG(absl::StrFormat("[                OrbitService] %s\n", line).c_str());
    }
  }
};

// This function makes it easy to execute a function object on a different thread in a synchronous
// way.
//
// While waiting for the function to finish executing on a different thread a Qt event loop
// processes other (UI-) events. The thread is determined by the associated thread of the QObject
// context.
template <typename Func>
void DeferToBackgroundThreadAndWait(QObject* context, Func&& func) {
  QEventLoop waiting_loop;  // This event loop processes main thread events while we wait for the
                            // background thread to finish executing func();

  QMetaObject::invokeMethod(
      context, [func = std::forward<Func>(func), waiting_loop = QPointer{&waiting_loop}]() mutable {
        func();
        if (waiting_loop) QMetaObject::invokeMethod(waiting_loop, &QEventLoop::quit);
      });

  waiting_loop.exec();
}

}  // namespace

template <typename T>
static outcome::result<T> MapError(outcome::result<T> result, Error new_error) {
  if (result) {
    return result;
  } else {
    const auto new_error_code = make_error_code(new_error);
    ERROR("%s: %s", new_error_code.message().c_str(), result.error().message().c_str());
    return outcome::failure(new_error_code);
  }
}

ServiceDeployManager::ServiceDeployManager(const DeploymentConfiguration* deployment_configuration,
                                           const orbit_ssh::Context* context,
                                           orbit_ssh::Credentials credentials,
                                           const ServiceDeployManager::GrpcPort& grpc_port,
                                           QObject* parent)
    : QObject(parent),
      deployment_configuration_(deployment_configuration),
      context_(context),
      credentials_(std::move(credentials)),
      grpc_port_(grpc_port),
      ssh_watchdog_timer_(this) {
  CHECK(deployment_configuration != nullptr);
  CHECK(context != nullptr);

  background_thread_.start();
  moveToThread(&background_thread_);
}

ServiceDeployManager::~ServiceDeployManager() noexcept {
  // ssh_watchdog_timer is registered in background_thread_, so it has to be stopped there to
  // not trigger a race condition.
  QMetaObject::invokeMethod(
      this, [this]() { ssh_watchdog_timer_.stop(); }, Qt::BlockingQueuedConnection);
  background_thread_.quit();
  background_thread_.wait();
}

void ServiceDeployManager::Cancel() {
  // By transforming this function call into a signal we leverage Qt's automatic thread
  // synchronization and don't have to bother from what thread Cancel was called.
  emit cancelRequested();
}

outcome::result<bool> ServiceDeployManager::CheckIfInstalled() {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage(QString("Checking if OrbitService is already installed in version %1 on the "
                             "remote instance.")
                         .arg(QApplication::applicationVersion()));

  auto version = QApplication::applicationVersion().toStdString();
  if (!version.empty() && version.front() == 'v') {
    // The old git tags have a 'v' in front which is not supported by debian
    // packages. So we have to remove it.
    version = version.substr(1);
  }
  const auto command = absl::StrFormat(
      "/usr/bin/dpkg-query -W -f '${Version}' orbitprofiler 2>/dev/null | grep -xF '%s'", version);

  orbit_ssh_qt::Task check_if_installed_task{&session_.value(), command};

  orbit_qt_utils::EventLoop loop{};
  QObject::connect(&check_if_installed_task, &orbit_ssh_qt::Task::finished, &loop,
                   &orbit_qt_utils::EventLoop::exit);

  auto error_handler =
      ConnectErrorHandler(&loop, &check_if_installed_task, &orbit_ssh_qt::Task::errorOccurred);

  auto cancel_handler = ConnectCancelHandler(&loop, this);

  check_if_installed_task.Start();

  OUTCOME_TRY(result, loop.exec());
  if (result == 0) {
    // Already installed
    emit statusMessage("The correct version of OrbitService is already installed.");
    return outcome::success(true);
  } else {
    emit statusMessage("The correct version of OrbitService is not yet installed.");
    return outcome::success(false);
  }
}

outcome::result<uint16_t> ServiceDeployManager::StartTunnel(
    std::optional<orbit_ssh_qt::Tunnel>* tunnel, uint16_t port) {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Setting up port forwarding...");
  LOG("Setting up tunnel on port %d", port);

  tunnel->emplace(&session_.value(), kLocalhost, port, this);

  orbit_qt_utils::EventLoop loop{};
  auto error_handler =
      ConnectErrorHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::errorOccurred);
  auto quit_handler = ConnectQuitHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::started);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  tunnel->value().Start();

  OUTCOME_TRY(MapError(loop.exec(), Error::kCouldNotStartTunnel));

  QObject::connect(&tunnel->value(), &orbit_ssh_qt::Tunnel::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success(tunnel->value().GetListenPort());
}

outcome::result<std::unique_ptr<orbit_ssh_qt::SftpChannel>>
ServiceDeployManager::StartSftpChannel() {
  CHECK(QThread::currentThread() == thread());
  auto sftp_channel = std::make_unique<orbit_ssh_qt::SftpChannel>(&session_.value());

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler =
      ConnectQuitHandler(&loop, sftp_channel.get(), &orbit_ssh_qt::SftpChannel::started);

  auto error_handler =
      ConnectErrorHandler(&loop, sftp_channel.get(), &orbit_ssh_qt::SftpChannel::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  sftp_channel->Start();

  OUTCOME_TRY(loop.exec());
  return sftp_channel;
}

outcome::result<void> ServiceDeployManager::CopyFileToRemote(
    const std::string& source, const std::string& dest,
    orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode dest_mode) {
  CHECK(QThread::currentThread() == thread());
  orbit_ssh_qt::SftpCopyToRemoteOperation operation{&session_.value(), sftp_channel_.get()};

  orbit_qt_utils::EventLoop loop{};

  auto quit_handler =
      ConnectQuitHandler(&loop, &operation, &orbit_ssh_qt::SftpCopyToRemoteOperation::stopped);

  auto error_handler = ConnectErrorHandler(&loop, &operation,
                                           &orbit_ssh_qt::SftpCopyToRemoteOperation::errorOccurred);

  auto cancel_handler = ConnectCancelHandler(&loop, this);

  LOG("About to start copying from %s to %s...", source, dest);
  operation.CopyFileToRemote(source, dest, dest_mode);

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StopSftpChannel(
    orbit_ssh_qt::SftpChannel* sftp_channel) {
  CHECK(QThread::currentThread() == thread());

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, sftp_channel, &orbit_ssh_qt::SftpChannel::stopped);
  auto error_handler =
      ConnectErrorHandler(&loop, sftp_channel, &orbit_ssh_qt::SftpChannel::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  sftp_channel->Stop();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

void ServiceDeployManager::StopSftpChannel() { (void)StopSftpChannel(sftp_channel_.get()); }

outcome::result<void> ServiceDeployManager::CopyOrbitServicePackage() {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying OrbitService package to the remote instance...");

  auto& config = std::get<SignedDebianPackageDeployment>(*deployment_configuration_);

  using FileMode = orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode;

  OUTCOME_TRY(MapError(CopyFileToRemote(config.path_to_package.string(), kDebDestinationPath,
                                        FileMode::kUserWritable),
                       Error::kCouldNotUploadPackage));

  OUTCOME_TRY(MapError(CopyFileToRemote(config.path_to_signature.string(), kSigDestinationPath,
                                        FileMode::kUserWritable),
                       Error::kCouldNotUploadSignature));

  emit statusMessage("Finished copying the OrbitService package to the remote instance.");
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::CopyFileToLocal(std::string source,
                                                           std::string destination) {
  orbit_base::Promise<ErrorMessageOr<void>> promise;
  auto future = promise.GetFuture();

  DeferToBackgroundThreadAndWait(
      this, [this, source = std::move(source), destination = std::move(destination),
             promise = std::move(promise)]() mutable {
        promise.SetResult(CopyFileToLocalImpl(source, destination));
      });

  if (!future.IsFinished()) return ErrorMessage{"Copy operation was aborted."};

  return future.Get();
}

ErrorMessageOr<void> ServiceDeployManager::CopyFileToLocalImpl(std::string_view source,
                                                               std::string_view destination) {
  CHECK(QThread::currentThread() == thread());
  LOG("Copying remote \"%s\" to local \"%s\"", source, destination);

  auto sftp_channel = StartSftpChannel();

  if (!sftp_channel) {
    return ErrorMessage(
        absl::StrFormat(R"(Unable to start sftp channel to copy the remote "%s" to "%s": %s)",
                        source, destination, sftp_channel.error().message()));
  }

  orbit_ssh_qt::SftpCopyToLocalOperation operation{&session_.value(), sftp_channel.value().get()};

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler =
      ConnectQuitHandler(&loop, &operation, &orbit_ssh_qt::SftpCopyToLocalOperation::stopped);
  auto error_handler = ConnectErrorHandler(&loop, &operation,
                                           &orbit_ssh_qt::SftpCopyToLocalOperation::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  operation.CopyFileToLocal(source, destination);

  auto result = loop.exec();
  if (!result) {
    return ErrorMessage(absl::StrFormat(R"(Error copying remote "%s" to "%s": %s)", source,
                                        destination, result.error().message()));
  }

  auto sftp_channel_stop_result = StopSftpChannel(sftp_channel.value().get());

  if (!sftp_channel_stop_result) {
    std::string sftp_error_message =
        absl::StrFormat(R"(Error closing sftp channel (after copied remote "%s" to "%s": %s))",
                        source, destination, sftp_channel_stop_result.error().message());
    ERROR("%s", sftp_error_message);
    return ErrorMessage(
        absl::StrFormat("Download of file %s failed: %s", source, sftp_error_message));
  }

  return outcome::success();
}

outcome::result<void> ServiceDeployManager::CopyOrbitServiceExecutable(
    const BareExecutableAndRootPasswordDeployment& config) {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying OrbitService executable to the remote instance...");

  const std::string exe_destination_path = "/tmp/OrbitService";
  OUTCOME_TRY(CopyFileToRemote(
      config.path_to_executable.string(), exe_destination_path,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritableAllExecutable));

  emit statusMessage("Finished copying the OrbitService executable to the remote instance.");
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::CopyOrbitApiLibrary(
    const BareExecutableAndRootPasswordDeployment& config) {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying liborbit.so to the remote instance...");

  const std::string library_destination_path = "/tmp/liborbit.so";
  const auto library_source_path = config.path_to_executable.parent_path() / "../lib/liborbit.so";
  OUTCOME_TRY(CopyFileToRemote(
      library_source_path.string(), library_destination_path,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritableAllExecutable));

  emit statusMessage("Finished copying liborbit.so to the remote instance.");
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StartOrbitService() {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Starting OrbitService on the remote instance...");

  std::string task_string = "/opt/developer/tools/OrbitService";
  if (absl::GetFlag(FLAGS_devmode)) {
    task_string += " --devmode";
  }
  orbit_service_task_.emplace(&session_.value(), task_string);

  orbit_qt_utils::EventLoop loop{};

  auto quit_handler =
      ConnectQuitHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::started);

  auto error_handler =
      ConnectErrorHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred);

  auto cancel_handler = ConnectCancelHandler(&loop, this);

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdOut, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdOut()); });

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdErr, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdErr()); });

  orbit_service_task_->Start();

  OUTCOME_TRY(loop.exec());
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::StartOrbitServicePrivileged(
    const BareExecutableAndRootPasswordDeployment& config) {
  CHECK(QThread::currentThread() == thread());
  // TODO(antonrohr) Check whether the password was incorrect.
  // There are multiple ways of doing this. the best way is probably to have a
  // second task running before OrbitService that sets the SUID bit. It might be
  // necessary to close stdin by sending EOF, since sudo would ask for trying to
  // enter the password again. Another option is to use std err as soon as its
  // implemented in OrbitSshQt::Task.
  emit statusMessage("Starting OrbitService on the remote instance...");

  std::string task_string = "sudo --stdin /tmp/OrbitService";
  if (absl::GetFlag(FLAGS_devmode)) {
    task_string += " --devmode";
  }
  orbit_service_task_.emplace(&session_.value(), task_string);

  orbit_service_task_->Write(absl::StrFormat("%s\n", config.root_password));

  orbit_qt_utils::EventLoop loop{};
  auto error_handler =
      ConnectErrorHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred);
  auto quit_handler =
      ConnectQuitHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::started);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdOut, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdOut()); });
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdErr, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdErr()); });

  orbit_service_task_->Start();

  OUTCOME_TRY(loop.exec());
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::InstallOrbitServicePackage() {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage("Installing the OrbitService package on the remote instance...");

  const auto command = absl::StrFormat(
      "sudo /usr/local/cloudcast/sbin/install_signed_package.sh %s", kDebDestinationPath);
  orbit_ssh_qt::Task install_service_task{&session_.value(), command};

  orbit_qt_utils::EventLoop loop{};

  QObject::connect(&install_service_task, &orbit_ssh_qt::Task::finished, this, [&](int exit_code) {
    if (exit_code == 0) {
      loop.quit();
    } else {
      // TODO(antonrohr) use stderr message once its implemented in
      // OrbitSshQt::Task
      ERROR("Unable to install install OrbitService package, exit code: %d", exit_code);
      loop.error(make_error_code(Error::kCouldNotInstallPackage));
    }
  });

  auto error_handler =
      ConnectErrorHandler(&loop, &install_service_task, &orbit_ssh_qt::Task::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  install_service_task.Start();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

outcome::result<void> ServiceDeployManager::ConnectToServer() {
  CHECK(QThread::currentThread() == thread());
  emit statusMessage(QString("Connecting to %1:%2...")
                         .arg(QString::fromStdString(credentials_.addr_and_port.addr))
                         .arg(credentials_.addr_and_port.port));

  session_.emplace(context_, this);

  using orbit_ssh_qt::Session;

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, &session_.value(), &Session::started);
  auto error_handler = ConnectErrorHandler(&loop, &session_.value(), &Session::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  session_->ConnectToServer(credentials_);

  OUTCOME_TRY(MapError(loop.exec(), Error::kCouldNotConnectToServer));

  emit statusMessage(QString("Successfully connected to %1:%2.")
                         .arg(QString::fromStdString(credentials_.addr_and_port.addr))
                         .arg(credentials_.addr_and_port.port));

  QObject::connect(&session_.value(), &Session::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

void ServiceDeployManager::StartWatchdog() {
  CHECK(QThread::currentThread() == thread());
  orbit_service_task_->Write(kSshWatchdogPassphrase);

  QObject::connect(&ssh_watchdog_timer_, &QTimer::timeout, [this]() {
    if (orbit_service_task_) {
      orbit_service_task_->Write(".");
    } else {
      ssh_watchdog_timer_.stop();
    }
  });

  ssh_watchdog_timer_.start(kSshWatchdogInterval);
}

outcome::result<ServiceDeployManager::GrpcPort> ServiceDeployManager::Exec() {
  outcome::result<GrpcPort> result = outcome::success(GrpcPort{0});
  DeferToBackgroundThreadAndWait(this, [&]() { result = ExecImpl(); });
  return result;
}

outcome::result<ServiceDeployManager::GrpcPort> ServiceDeployManager::ExecImpl() {
  CHECK(QThread::currentThread() == thread());
  OUTCOME_TRY(ConnectToServer());

  OUTCOME_TRY(sftp_channel, StartSftpChannel());
  sftp_channel_ = std::move(sftp_channel);
  // Release mode: Deploying a signed debian package. No password required.
  if (std::holds_alternative<SignedDebianPackageDeployment>(*deployment_configuration_)) {
    OUTCOME_TRY(service_already_installed, CheckIfInstalled());

    if (!service_already_installed) {
      OUTCOME_TRY(CopyOrbitServicePackage());
      OUTCOME_TRY(InstallOrbitServicePackage());
    }
    OUTCOME_TRY(StartOrbitService());
    // TODO(hebecker): Replace this timeout by waiting for a
    //  stdout-greeting-message.
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    StartWatchdog();

    // Developer mode: Deploying a bare executable and start it via sudo.
  } else if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(
                 *deployment_configuration_)) {
    const auto& config =
        std::get<BareExecutableAndRootPasswordDeployment>(*deployment_configuration_);
    OUTCOME_TRY(CopyOrbitServiceExecutable(config));
    OUTCOME_TRY(CopyOrbitApiLibrary(config));
    OUTCOME_TRY(StartOrbitServicePrivileged(config));
    // TODO(hebecker): Replace this timeout by waiting for a
    // stdout-greeting-message.
    std::this_thread::sleep_for(std::chrono::milliseconds{200});

    StartWatchdog();

    // Manual Developer mode: No deployment, no starting. Just the tunnels.
  } else if (std::holds_alternative<NoDeployment>(*deployment_configuration_)) {
    // Nothing to deploy
    emit statusMessage(
        "Skipping deployment step. Expecting that OrbitService is already "
        "running...");
  }

  outcome::result<uint16_t> local_grpc_port_result =
      StartTunnel(&grpc_tunnel_, grpc_port_.grpc_port);
  int retry = 3;
  while (retry > 0 && local_grpc_port_result.has_error()) {
    ERROR("Failed to establish tunnel. Trying again in 500ms");
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    local_grpc_port_result = StartTunnel(&grpc_tunnel_, grpc_port_.grpc_port);
    retry--;
  }

  OUTCOME_TRY(local_grpc_port, local_grpc_port_result);

  emit statusMessage("Successfully set up port forwarding!");

  LOG("Local port for gRPC is %d", local_grpc_port);
  return outcome::success(GrpcPort{local_grpc_port});
}

void ServiceDeployManager::handleSocketError(std::error_code e) {
  LOG("Socket error: %s", e.message());
  emit socketErrorOccurred(e);
}

void ServiceDeployManager::ShutdownTunnel(std::optional<orbit_ssh_qt::Tunnel>* tunnel) {
  if (!tunnel || !*tunnel) {
    return;
  }

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::started);
  auto error_handler =
      ConnectQuitHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  tunnel->value().Stop();

  (void)loop.exec();
  *tunnel = std::nullopt;
}

void ServiceDeployManager::ShutdownOrbitService() {
  if (!orbit_service_task_) {
    return;
  }

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler =
      ConnectQuitHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::finished);
  auto error_handler =
      ConnectQuitHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  orbit_service_task_->Stop();

  (void)loop.exec();
  orbit_service_task_ = std::nullopt;
}

void ServiceDeployManager::ShutdownSession() {
  if (!session_) {
    return;
  }

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, &session_.value(), &orbit_ssh_qt::Session::stopped);
  auto error_handler =
      ConnectQuitHandler(&loop, &session_.value(), &orbit_ssh_qt::Session::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  session_->Disconnect();

  (void)loop.exec();
  session_ = std::nullopt;
}

void ServiceDeployManager::Shutdown() {
  DeferToBackgroundThreadAndWait(this, [this]() {
    StopSftpChannel();
    ShutdownTunnel(&grpc_tunnel_);
    ShutdownOrbitService();
    ShutdownSession();
  });
}

}  // namespace orbit_session_setup
