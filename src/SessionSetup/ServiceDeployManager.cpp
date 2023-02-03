// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ServiceDeployManager.h"

#include <absl/flags/flag.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <QApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <QNonConstOverload>
#include <QPointer>
#include <Qt>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "ClientFlags/ClientFlags.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToLocalOperation.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "QtUtils/EventLoop.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/Error.h"

static const std::string kLocalhost = "127.0.0.1";
static const std::string kDebDestinationPath = "/tmp/orbitprofiler.deb";
static const std::string kSigDestinationPath = "/tmp/orbitprofiler.deb.asc";
constexpr std::string_view kSshWatchdogPassphrase = "start_watchdog";
constexpr std::chrono::milliseconds kSshWatchdogInterval{1000};
constexpr std::chrono::seconds kServiceStartupTimeout{10};

using orbit_base::CanceledOr;

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
  return orbit_ssh_qt::ScopedConnection{QObject::connect(
      sender, signal, loop, qOverload<std::error_code>(&orbit_qt_utils::EventLoop::error))};
}

[[nodiscard]] orbit_ssh_qt::ScopedConnection ConnectCancelHandler(orbit_qt_utils::EventLoop* loop,
                                                                  ServiceDeployManager* sdm) {
  return orbit_ssh_qt::ScopedConnection{QObject::connect(
      sdm, &ServiceDeployManager::cancelRequested, loop,
      [loop]() { loop->error(make_error_code(Error::kUserCanceledServiceDeployment)); })};
}

void PrintAsOrbitService(std::string_view buffer) {
  std::vector<std::string_view> lines = absl::StrSplit(buffer, '\n');
  for (const auto& line : lines) {
    if (!line.empty()) {
      ORBIT_INTERNAL_PLATFORM_LOG(
          absl::StrFormat("[                OrbitService] %s\n", line).c_str());
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

  QMetaObject::invokeMethod(context,
                            [func = std::forward<Func>(func),
                             waiting_loop = QPointer<QEventLoop>{&waiting_loop}]() mutable {
                              func();
                              if (waiting_loop)
                                QMetaObject::invokeMethod(waiting_loop, &QEventLoop::quit);
                            });

  waiting_loop.exec();
}

}  // namespace

template <typename T>
static ErrorMessageOr<T> MapError(ErrorMessageOr<T> result, Error new_error) {
  if (result.has_value()) {
    return result;
  }
  const auto new_error_code = make_error_code(new_error);
  ORBIT_ERROR("%s: %s", new_error_code.message(), result.error().message());
  return outcome::failure(new_error_code);
}

template <typename T>
static ErrorMessageOr<T> MapError(outcome::result<T> result, Error new_error) {
  if (result.has_value()) {
    return result.value();
  }
  const auto new_error_code = make_error_code(new_error);
  return ErrorMessage{
      absl::StrFormat("%s: %s", new_error_code.message(), result.error().message())};
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
  ORBIT_CHECK(deployment_configuration != nullptr);
  ORBIT_CHECK(context != nullptr);

  moveToThread(background_executor_.GetThread());

  QObject::connect(
      this, &ServiceDeployManager::statusMessage, this, [](const QString& status_message) {
        ORBIT_LOG("ServiceDeployManager status message: \"%s\"", status_message.toStdString());
      });
}

ServiceDeployManager::~ServiceDeployManager() { Shutdown(); }

void ServiceDeployManager::Cancel() {
  // By transforming this function call into a signal we leverage Qt's automatic thread
  // synchronization and don't have to bother from what thread Cancel was called.
  emit cancelRequested();
}

ErrorMessageOr<bool> ServiceDeployManager::CheckIfInstalled() {
  ORBIT_CHECK(QThread::currentThread() == thread());
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
      "/usr/bin/dpkg-query -W -f '${Version}' orbitprofiler | grep -xF '%s' && cd / && md5sum -c "
      "/var/lib/dpkg/info/orbitprofiler.md5sums",
      version);

  orbit_ssh_qt::Task check_if_installed_task{&session_.value(), command};

  orbit_qt_utils::EventLoop loop{};
  QObject::connect(&check_if_installed_task, &orbit_ssh_qt::Task::readyReadStdOut, this,
                   [&check_if_installed_task]() {
                     ORBIT_LOG("CheckIfInstalled stdout: %s", check_if_installed_task.ReadStdOut());
                   });
  QObject::connect(&check_if_installed_task, &orbit_ssh_qt::Task::readyReadStdErr, this,
                   [&check_if_installed_task]() {
                     ORBIT_LOG("CheckIfInstalled stderr: %s", check_if_installed_task.ReadStdErr());
                   });
  QObject::connect(&check_if_installed_task, &orbit_ssh_qt::Task::finished, &loop,
                   &orbit_qt_utils::EventLoop::exit);

  auto error_handler =
      ConnectErrorHandler(&loop, &check_if_installed_task, &orbit_ssh_qt::Task::errorOccurred);

  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = check_if_installed_task.Start();

  OUTCOME_TRY(auto&& result, loop.exec());
  ORBIT_LOG("CheckIfInstalled task returned exit code: %d", result);
  if (result == 0) {
    // Already installed
    emit statusMessage("The correct version of OrbitService is already installed.");
    return outcome::success(true);
  }
  emit statusMessage("The correct version of OrbitService is not yet installed.");
  return outcome::success(false);
}

ErrorMessageOr<uint16_t> ServiceDeployManager::StartTunnel(
    std::optional<orbit_ssh_qt::Tunnel>* tunnel, uint16_t port) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Setting up port forwarding...");
  ORBIT_LOG("Setting up tunnel on port %d", port);

  tunnel->emplace(&session_.value(), kLocalhost, port, this);

  orbit_qt_utils::EventLoop loop{};
  auto error_handler =
      ConnectErrorHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::errorOccurred);
  auto quit_handler = ConnectQuitHandler(&loop, &tunnel->value(), &orbit_ssh_qt::Tunnel::started);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = tunnel->value().Start();

  OUTCOME_TRY(MapError(loop.exec(), Error::kCouldNotStartTunnel));

  QObject::connect(&tunnel->value(), &orbit_ssh_qt::Tunnel::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success(tunnel->value().GetListenPort());
}

ErrorMessageOr<std::unique_ptr<orbit_ssh_qt::SftpChannel>>
ServiceDeployManager::StartSftpChannel() {
  ORBIT_CHECK(QThread::currentThread() == thread());
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

ErrorMessageOr<void> ServiceDeployManager::CopyFileToRemote(
    std::string_view source, std::string_view dest,
    orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode dest_mode) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  orbit_ssh_qt::SftpCopyToRemoteOperation operation{&session_.value(), sftp_channel_.get()};

  orbit_qt_utils::EventLoop loop{};

  auto quit_handler =
      ConnectQuitHandler(&loop, &operation, &orbit_ssh_qt::SftpCopyToRemoteOperation::stopped);

  auto error_handler = ConnectErrorHandler(&loop, &operation,
                                           &orbit_ssh_qt::SftpCopyToRemoteOperation::errorOccurred);

  auto cancel_handler = ConnectCancelHandler(&loop, this);

  ORBIT_LOG("About to start copying from %s to %s...", source, dest);
  operation.CopyFileToRemote(source, dest, dest_mode);

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::ShutdownSftpOperations() {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::ShutdownSftpOperations");
  ORBIT_CHECK(QThread::currentThread() == thread());
  ORBIT_CHECK(copy_to_local_operation_ != nullptr);

  waiting_copy_operations_.clear();

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, copy_to_local_operation_,
                                         &orbit_ssh_qt::SftpCopyToLocalOperation::stopped);
  auto error_handler = ConnectErrorHandler(&loop, copy_to_local_operation_,
                                           &orbit_ssh_qt::SftpCopyToLocalOperation::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  copy_to_local_operation_->Stop();
  OUTCOME_TRY(loop.exec());

  delete copy_to_local_operation_;
  copy_to_local_operation_ = nullptr;

  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::ShutdownSftpChannel(
    orbit_ssh_qt::SftpChannel* sftp_channel) {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::ShutdownSftpChannel");
  ORBIT_CHECK(QThread::currentThread() == thread());
  ORBIT_CHECK(sftp_channel != nullptr);

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, sftp_channel, &orbit_ssh_qt::SftpChannel::stopped);
  auto error_handler =
      ConnectErrorHandler(&loop, sftp_channel, &orbit_ssh_qt::SftpChannel::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  sftp_channel->Stop();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::CopyOrbitServicePackage() {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying OrbitService package to the remote instance...");

  const auto& config = std::get<SignedDebianPackageDeployment>(*deployment_configuration_);

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

orbit_base::Future<ErrorMessageOr<CanceledOr<void>>> ServiceDeployManager::CopyFileToLocal(
    std::filesystem::path source, std::filesystem::path destination,
    orbit_base::StopToken stop_token) {
  orbit_base::Promise<ErrorMessageOr<CanceledOr<void>>> promise;
  auto future = promise.GetFuture();

  // This schedules the call of `CopyFileToLocalImpl` on the background thread.
  QMetaObject::invokeMethod(
      this, [this, source = std::move(source), destination = std::move(destination),
             promise = std::move(promise), stop_token = std::move(stop_token)]() mutable {
        CopyFileToLocalImpl(std::move(promise), std::move(source), std::move(destination),
                            std::move(stop_token));
      });

  return future;
}

void ServiceDeployManager::CopyFileToLocalImpl(
    orbit_base::Promise<ErrorMessageOr<CanceledOr<void>>> promise, std::filesystem::path source,
    std::filesystem::path destination, orbit_base::StopToken stop_token) {
  ORBIT_CHECK(QThread::currentThread() == thread());

  if (copy_to_local_operation_ != nullptr) {
    waiting_copy_operations_.emplace_back(
        [this, promise = std::move(promise), source = std::move(source),
         destination = std::move(destination), stop_token = std::move(stop_token)]() mutable {
          CopyFileToLocalImpl(std::move(promise), source, destination, std::move(stop_token));
        });
    return;
  }

  ORBIT_LOG("Copying remote \"%s\" to local \"%s\"", source.string(), destination.string());

  // NOLINTNEXTLINE - Unfortunately we have to fall back to a raw `new` here.
  copy_to_local_operation_ = new orbit_ssh_qt::SftpCopyToLocalOperation{
      &session_.value(), sftp_channel_.get(), stop_token};
  // copy_to_local_operation_ will get deleted either in finish_handler (via delete later) or in
  // ShutdownSftpOperations().

  // The finish handler handles both the error and the success case and will be triggered
  // from the ::stopped and ::errorOccured signals (see below).
  // By having a single handler we don't need to worry about sharing resources that are not supposed
  // to be shared like the promise.
  auto finish_handler = [this, promise = std::move(promise), source, destination,
                         stop_token = std::move(stop_token)](ErrorMessageOr<void> result) mutable {
    if (promise.HasResult()) return;

    // We can't just call `delete copy_to_local_operation_;` here because that also triggers the
    // deletion of this closure object. Instead we queue a job on the event queue for deleting it
    // later.
    copy_to_local_operation_->deleteLater();
    copy_to_local_operation_ = nullptr;

    if (!waiting_copy_operations_.empty()) {
      // This calls the copy operation from the event loop in the background thread
      QMetaObject::invokeMethod(this, std::move(waiting_copy_operations_.front()),
                                Qt::QueuedConnection);
      waiting_copy_operations_.pop_front();
    }

    if (stop_token.IsStopRequested()) {
      promise.SetResult(orbit_base::Canceled{});
      return;
    }

    if (result.has_error()) {
      promise.SetResult(
          ErrorMessage{absl::StrFormat(R"(Error copying remote "%s" to "%s": %s)", source.string(),
                                       destination.string(), result.error().message())});
      return;
    }

    promise.SetResult(outcome::success());
  };

  // Since we need to call the finish handler from two different slots and it's not copyable,
  // we first have to move the handler into a shared_ptr which we can share between the two slots.
  // This will also take care of lifetime management since the finish handler closure will get
  // deleted when the `copy_to_local_operation_` object gets deleted.
  auto shared_finish_handler =
      std::make_shared<decltype(finish_handler)>(std::move(finish_handler));

  QObject::connect(copy_to_local_operation_, &orbit_ssh_qt::SftpCopyToLocalOperation::stopped,
                   [shared_finish_handler]() { (*shared_finish_handler)(outcome::success()); });

  QObject::connect(copy_to_local_operation_, &orbit_ssh_qt::SftpCopyToLocalOperation::errorOccurred,
                   [shared_finish_handler](std::error_code error_code) {
                     (*shared_finish_handler)(ErrorMessage{error_code.message()});
                   });

  copy_to_local_operation_->CopyFileToLocal(std::move(source), std::move(destination));
}

ErrorMessageOr<void> ServiceDeployManager::CopyOrbitServiceExecutable(
    const BareExecutableAndRootPasswordDeployment& config) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying OrbitService executable to the remote instance...");

  const std::string exe_destination_path = "/tmp/OrbitService";
  OUTCOME_TRY(CopyFileToRemote(
      config.path_to_executable.string(), exe_destination_path,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritableAllExecutable));

  emit statusMessage("Finished copying the OrbitService executable to the remote instance.");
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::CopyOrbitApiLibrary(
    const BareExecutableAndRootPasswordDeployment& config) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying liborbit.so to the remote instance...");

  const std::string library_destination_path = "/tmp/liborbit.so";
  const auto library_source_path = config.path_to_executable.parent_path() / "../lib/liborbit.so";
  OUTCOME_TRY(CopyFileToRemote(
      library_source_path.string(), library_destination_path,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritableAllExecutable));

  emit statusMessage("Finished copying liborbit.so to the remote instance.");
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::CopyOrbitUserSpaceInstrumentationLibrary(
    const BareExecutableAndRootPasswordDeployment& config) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Copying liborbituserspaceinstrumentation.so to the remote instance...");

  const std::string library_destination_path = "/tmp/liborbituserspaceinstrumentation.so";
  const auto library_source_path =
      config.path_to_executable.parent_path() / "../lib/liborbituserspaceinstrumentation.so";
  OUTCOME_TRY(CopyFileToRemote(
      library_source_path.string(), library_destination_path,
      orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritableAllExecutable));

  emit statusMessage(
      "Finished copying liborbituserspaceinstrumentation.so to the remote instance.");
  return outcome::success();
}

[[nodiscard]] static std::string GenerateStartOrbitServiceCommand(
    const std::variant<SignedDebianPackageDeployment, BareExecutableAndRootPasswordDeployment>&
        deployment_config) {
  std::string command;
  if (std::holds_alternative<SignedDebianPackageDeployment>(deployment_config)) {
    command = "/opt/developer/tools/OrbitService";
  } else {
    command = "sudo --stdin /tmp/OrbitService";
  }

  if (absl::GetFlag(FLAGS_devmode)) {
    command += " --devmode";
  }

  return command;
}

ErrorMessageOr<void> ServiceDeployManager::StartOrbitService(
    const std::variant<SignedDebianPackageDeployment, BareExecutableAndRootPasswordDeployment>&
        deployment_config) {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage("Starting OrbitService on the remote instance...");

  std::string task_string = GenerateStartOrbitServiceCommand(deployment_config);
  orbit_service_task_.emplace(&session_.value(), task_string);

  if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(deployment_config)) {
    const auto& config = std::get<BareExecutableAndRootPasswordDeployment>(deployment_config);
    std::ignore = orbit_service_task_->Write(absl::StrFormat("%s\n", config.root_password));
    // TODO(antonrohr) Check whether the password was incorrect.
    // There are multiple ways of doing this. the best way is probably to have a
    // second task running before OrbitService that sets the SUID bit. It might be
    // necessary to close stdin by sending EOF, since sudo would ask for trying to
    // enter the password again. Another option is to use std err as soon as its
    // implemented in OrbitSshQt::Task.
  }

  orbit_qt_utils::EventLoop loop{};
  auto error_handler =
      ConnectErrorHandler(&loop, &orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::string stdout_buffer;

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::finished, &loop,
                   [&loop, &stdout_buffer](int exit_code) {
                     constexpr int kExitCodeIndicatingErrorMessage = 42;
                     if (exit_code == kExitCodeIndicatingErrorMessage) {
                       // We convert to QString here because there could be UTF-8 multibyte
                       // codepoints in the stdout_buffer which makes limiting to a certain number
                       // of characters non-trivial.
                       auto error_message = QString::fromStdString(stdout_buffer).trimmed();
                       constexpr int kMaximumErrorMessageLength = 1000;
                       if (error_message.size() > kMaximumErrorMessageLength) {
                         error_message =
                             error_message.left(kMaximumErrorMessageLength - 3).append("...");
                       }
                       loop.error(ErrorMessage{error_message.toStdString()});
                       return;
                     }

                     loop.error(ErrorMessage{absl::StrFormat(
                         "The service exited prematurely with exit code %d.", exit_code)});
                   });

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdOut, &loop,
                   [&]() {
                     // We are looking for the kReadyKeyword. Since it might be split up into
                     // consecutive chunks in the stdout stream we reassemble to whole string into a
                     // buffer and check that for the keyword.
                     stdout_buffer.append(orbit_service_task_->ReadStdOut());

                     // That's what we expect the service to send through stdout when it's ready to
                     // accept a connection from the client.
                     constexpr std::string_view kReadyKeyword = "READY";

                     if (absl::StrContains(stdout_buffer, kReadyKeyword)) {
                       ORBIT_LOG("The service reported to be ready to accept connections.");
                       loop.quit();
                       return;
                     }

                     // This is protecting us against consuming unreasonable amount of memory when
                     // for whatever reason there is a lot of data coming through the stdout
                     // channel.
                     constexpr size_t kMaxBufferSize = 100ul * 1024;  // 100 KiB

                     if (stdout_buffer.size() > kMaxBufferSize) {
                       const auto number_of_bytes_to_remove =
                           static_cast<ptrdiff_t>(stdout_buffer.size() - kMaxBufferSize);
                       stdout_buffer.erase(stdout_buffer.begin(),
                                           stdout_buffer.begin() + number_of_bytes_to_remove);
                     }
                   });

  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdErr, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdErr()); });

  QTimer::singleShot(kServiceStartupTimeout, &loop, [&]() {
    // OrbitService took too long to start. That's an indication that something is wrong.
    std::string error_message = absl::StrFormat(
        "The service took more than %d seconds to start up.", kServiceStartupTimeout.count());

    if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(deployment_config)) {
      error_message.append(" (An outdated version of OrbitService could have caused this.)");
    }
    loop.error(ErrorMessage{std::move(error_message)});
  });

  std::ignore = orbit_service_task_->Start();

  OUTCOME_TRY(loop.exec());
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::readyReadStdOut, this,
                   [this]() { PrintAsOrbitService(orbit_service_task_->ReadStdOut()); });
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  QObject::connect(&orbit_service_task_.value(), &orbit_ssh_qt::Task::finished, this,
                   [](int exit_code) {
                     ORBIT_LOG("The OrbitService Task finished with exit code: %d", exit_code);
                   });
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::InstallOrbitServicePackage() {
  ORBIT_CHECK(QThread::currentThread() == thread());
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
      ORBIT_ERROR("Unable to install install OrbitService package, exit code: %d", exit_code);
      loop.error(make_error_code(Error::kCouldNotInstallPackage));
    }
  });

  auto error_handler =
      ConnectErrorHandler(&loop, &install_service_task, &orbit_ssh_qt::Task::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = install_service_task.Start();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::ConnectToServer() {
  ORBIT_CHECK(QThread::currentThread() == thread());
  emit statusMessage(QString("Connecting to %1:%2...")
                         .arg(QString::fromStdString(credentials_.addr_and_port.addr))
                         .arg(credentials_.addr_and_port.port));

  session_.emplace(context_, this);

  using orbit_ssh_qt::Session;

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, &session_.value(), &Session::started);
  auto error_handler = ConnectErrorHandler(&loop, &session_.value(), &Session::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = session_->ConnectToServer(credentials_);

  OUTCOME_TRY(MapError(loop.exec(), Error::kCouldNotConnectToServer));

  emit statusMessage(QString("Successfully connected to %1:%2.")
                         .arg(QString::fromStdString(credentials_.addr_and_port.addr))
                         .arg(credentials_.addr_and_port.port));

  QObject::connect(&session_.value(), &Session::errorOccurred, this,
                   &ServiceDeployManager::handleSocketError);
  return outcome::success();
}

void ServiceDeployManager::StartWatchdog() {
  ORBIT_CHECK(QThread::currentThread() == thread());
  std::ignore = orbit_service_task_->Write(kSshWatchdogPassphrase);

  QObject::connect(&ssh_watchdog_timer_, &QTimer::timeout, [this]() {
    ORBIT_CHECK(orbit_service_task_.has_value());
    std::ignore = orbit_service_task_->Write(".");
  });

  ssh_watchdog_timer_.start(kSshWatchdogInterval);
}

ErrorMessageOr<ServiceDeployManager::GrpcPort> ServiceDeployManager::Exec() {
  ErrorMessageOr<GrpcPort> result = outcome::success(GrpcPort{0});
  DeferToBackgroundThreadAndWait(this, [&]() { result = ExecImpl(); });

  if (!result.has_value()) {
    if (result.error() == make_error_code(Error::kUserCanceledServiceDeployment)) {
      ORBIT_LOG("OrbitService deployment has been aborted by the user");
    } else {
      ORBIT_ERROR("OrbitService deployment failed, error: %s", result.error().message());
    }
  } else {
    ORBIT_LOG("Deployment successful, grpc_port: %d", result.value().grpc_port);
  }

  return result;
}

ErrorMessageOr<ServiceDeployManager::GrpcPort> ServiceDeployManager::ExecImpl() {
  ORBIT_CHECK(QThread::currentThread() == thread());
  OUTCOME_TRY(ConnectToServer());

  OUTCOME_TRY(auto&& sftp_channel, StartSftpChannel());
  sftp_channel_ = std::move(sftp_channel);
  // Release mode: Deploying a signed debian package. No password required.
  if (std::holds_alternative<SignedDebianPackageDeployment>(*deployment_configuration_)) {
    const auto& config = std::get<SignedDebianPackageDeployment>(*deployment_configuration_);

    OUTCOME_TRY(auto&& service_already_installed, CheckIfInstalled());

    if (!service_already_installed) {
      OUTCOME_TRY(CopyOrbitServicePackage());
      OUTCOME_TRY(InstallOrbitServicePackage());
    }
    OUTCOME_TRY(StartOrbitService(config));
    StartWatchdog();

    // Developer mode: Deploying a bare executable and start it via sudo.
  } else if (std::holds_alternative<BareExecutableAndRootPasswordDeployment>(
                 *deployment_configuration_)) {
    const auto& config =
        std::get<BareExecutableAndRootPasswordDeployment>(*deployment_configuration_);
    OUTCOME_TRY(CopyOrbitServiceExecutable(config));
    OUTCOME_TRY(CopyOrbitApiLibrary(config));
    OUTCOME_TRY(CopyOrbitUserSpaceInstrumentationLibrary(config));
    OUTCOME_TRY(StartOrbitService(config));
    StartWatchdog();

    // Manual Developer mode: No deployment, no starting. Just the tunnels.
  } else if (std::holds_alternative<NoDeployment>(*deployment_configuration_)) {
    // Nothing to deploy
    emit statusMessage(
        "Skipping deployment step. Expecting that OrbitService is already "
        "running...");
  }

  ErrorMessageOr<uint16_t> local_grpc_port_result =
      StartTunnel(&grpc_tunnel_, grpc_port_.grpc_port);
  int retry = 3;
  while (retry > 0 && local_grpc_port_result.has_error()) {
    ORBIT_ERROR("Failed to establish tunnel. Trying again in 500ms");
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    local_grpc_port_result = StartTunnel(&grpc_tunnel_, grpc_port_.grpc_port);
    retry--;
  }

  OUTCOME_TRY(auto&& local_grpc_port, local_grpc_port_result);

  emit statusMessage("Successfully set up port forwarding!");

  ORBIT_LOG("Local port for gRPC is %d", local_grpc_port);
  return outcome::success(GrpcPort{local_grpc_port});
}

void ServiceDeployManager::handleSocketError(std::error_code e) {
  ORBIT_LOG("Socket error: %s", e.message());
  emit socketErrorOccurred(e);
}

ErrorMessageOr<void> ServiceDeployManager::ShutdownTunnel(orbit_ssh_qt::Tunnel* tunnel) {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::StopTunnel");
  ORBIT_CHECK(tunnel != nullptr);

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, tunnel, &orbit_ssh_qt::Tunnel::stopped);
  auto error_handler = ConnectQuitHandler(&loop, tunnel, &orbit_ssh_qt::Tunnel::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = tunnel->Stop();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::ShutdownTask(orbit_ssh_qt::Task* task) {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::ShutdownOrbitService");
  ORBIT_CHECK(task != nullptr);

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, task, &orbit_ssh_qt::Task::stopped);
  auto error_handler = ConnectQuitHandler(&loop, task, &orbit_ssh_qt::Task::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  std::ignore = task->Stop();

  OUTCOME_TRY(loop.exec());
  return outcome::success();
}

ErrorMessageOr<void> ServiceDeployManager::ShutdownSession(orbit_ssh_qt::Session* session) {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::ShutdownSession");
  ORBIT_CHECK(session != nullptr);

  orbit_qt_utils::EventLoop loop{};
  auto quit_handler = ConnectQuitHandler(&loop, session, &orbit_ssh_qt::Session::stopped);
  auto error_handler = ConnectQuitHandler(&loop, session, &orbit_ssh_qt::Session::errorOccurred);
  auto cancel_handler = ConnectCancelHandler(&loop, this);

  orbit_base::Future<ErrorMessageOr<void>> disconnect_future = session->Disconnect();

  if (!disconnect_future.IsFinished()) {
    OUTCOME_TRY(loop.exec());
  }

  return outcome::success();
}

void ServiceDeployManager::Shutdown() {
  ORBIT_SCOPED_TIMED_LOG("ServiceDeployManager::Shutdown");
  QMetaObject::invokeMethod(
      this,
      [this]() {
        if (copy_to_local_operation_ != nullptr) {
          ErrorMessageOr<void> shutdown_result = ShutdownSftpOperations();
          if (shutdown_result.has_error()) {
            ORBIT_ERROR("Unable to shut down ongoing copy to local operation: %s",
                        shutdown_result.error().message());
          }
        }
        if (sftp_channel_ != nullptr) {
          ErrorMessageOr<void> shutdown_result = ShutdownSftpChannel(sftp_channel_.get());
          if (shutdown_result.has_error()) {
            ORBIT_ERROR("Unable to ShutdownSftpChannel: %s", shutdown_result.error().message());
          }
          sftp_channel_.reset();
        }
        if (grpc_tunnel_.has_value()) {
          ErrorMessageOr<void> shutdown_result = ShutdownTunnel(&grpc_tunnel_.value());
          if (shutdown_result.has_error()) {
            ORBIT_ERROR("Unable to ShutdownTunnel: %s", shutdown_result.error().message());
          }
          grpc_tunnel_ = std::nullopt;
        }
        ssh_watchdog_timer_.stop();
        if (orbit_service_task_.has_value()) {
          ErrorMessageOr<void> shutdown_result = ShutdownTask(&orbit_service_task_.value());
          if (shutdown_result.has_error()) {
            ORBIT_ERROR("Unable to ShutdownTask: %s", shutdown_result.error().message());
          }
          orbit_service_task_ = std::nullopt;
        }
        if (session_.has_value()) {
          ErrorMessageOr<void> shutdown_result = ShutdownSession(&session_.value());
          if (shutdown_result.has_error()) {
            ORBIT_ERROR("Unable to ShutdownSession: %s", shutdown_result.error().message());
          }
          session_ = std::nullopt;
        }
      },
      Qt::BlockingQueuedConnection);
}

}  // namespace orbit_session_setup
