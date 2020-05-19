// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QCoreApplication>
#include <QEventLoop>
#include <QSocketNotifier>
#include <QTimer>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <thread>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpSshInfo.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/SessionManager.h"
#include "OrbitSsh/Sftp.h"
#include "OrbitSsh/SftpFile.h"
#include "OrbitSsh/Task.h"
#include "OrbitSsh/TunnelManager.h"

static std::optional<GgpSshInfo> GetSshInfoSync(int index) {
  GgpClient::ResultOrQString<GgpClient> create_result = GgpClient::Create();
  if (!create_result) {
    ERROR("Unable to use ggp command line, error: %s",
          create_result.error().toStdString().c_str());
    return std::nullopt;
  }
  GgpClient client = std::move(create_result.value());
  LOG("Created ggp client");

  QEventLoop loop{};
  std::optional<GgpSshInfo> opt_ssh_info = std::nullopt;
  client.GetInstancesAsync(
      [&loop, &client, &opt_ssh_info,
       &index](GgpClient::ResultOrQString<QVector<GgpInstance>> vector) {
        if (!vector) {
          ERROR("%s", vector.error().toStdString().c_str());
          loop.quit();
          return;
        }

        if (vector.value().empty()) {
          ERROR("no reserved instances");
          loop.quit();
          return;
        }

        LOG("Got ggp instances");
        const GgpInstance& instance{vector.value()[index]};

        client.GetSshInformationAsync(
            instance, [&loop, &opt_ssh_info](
                          GgpClient::ResultOrQString<GgpSshInfo> result) {
              if (!result) {
                ERROR("%s", result.error().toStdString().c_str());
                loop.quit();
                return;
              }

              LOG("Got ggp ssh init data");
              opt_ssh_info = result.value();
              loop.quit();
            });
      });

  loop.exec();

  if (!opt_ssh_info) {
    ERROR("Could not get ssh info of instance");
    return std::nullopt;
  }

  return std::move(opt_ssh_info).value();
}

/**
 * Calls `generator` over and over again until the given return value is truthy
 * or an error occurred. The implementation uses QEventLoop and QSocketNotifier
 * instead of a bare select() to show how integration works with a event loop.
 **/
template <typename F>
static auto WaitFor(qintptr fd, F generator) {
  auto result = generator();

  if (result) {
    return result;
  }

  QEventLoop loop{};

  const auto tick = [&]() {
    result = generator();

    if (result ||
        result.error() != OrbitSsh::make_error_code(OrbitSsh::Error::kEagain)) {
      loop.quit();
    }
  };

  QSocketNotifier notifier{fd, QSocketNotifier::Read};
  QObject::connect(&notifier, &QSocketNotifier::activated, &loop, tick);

  loop.exec();

  return result;
}

/**
 * Writes `data` to the handle `file`. Blocks until the write operation is done
 * or an error occurred. The implementation uses QEventLoop and QSocketNotifier
 * instead of a bare select() to show how integration works with a event loop.
 **/
static outcome::result<void> SyncWrite(qintptr fd, OrbitSsh::SftpFile* file,
                                       OrbitSsh::Session* session,
                                       std::string_view data) {
  QEventLoop loop{};

  outcome::result<void> result = outcome::success();

  const auto write = [&]() {
    auto tmp = file->Write(data);

    if (!tmp &&
        tmp.error() != OrbitSsh::make_error_code(OrbitSsh::Error::kEagain)) {
      char* msg;
      int length;
      libssh2_session_last_error(session->GetRawSessionPtr(), &msg, &length,
                                 false);
      LOG("ERR: %s", std::string_view{msg, static_cast<size_t>(length)});
      result = outcome::failure(tmp.error());
      loop.quit();
      return true;
    }

    if (tmp) {
      LOG("Written Bytes: %i", tmp.value());
      data = data.substr(tmp.value());

      if (data.empty()) {
        result = outcome::success();
        loop.quit();
        return true;
      }
    }

    return false;
  };

  if (write()) {
    return result;
  }

  QSocketNotifier notifier{fd, QSocketNotifier::Read};
  QObject::connect(&notifier, &QSocketNotifier::activated, &loop, write);

  loop.exec();

  return result;
}

/**
 * Read string from handle `file`. Blocks until no more data can be read or an
 * error occurred. The implementation uses QEventLoop and QSocketNotifier instead
 * of a bare select() to show how integration works with a event loop.
 **/
static outcome::result<std::string> SyncRead(qintptr fd,
                                             OrbitSsh::SftpFile* file,
                                             OrbitSsh::Session* session) {
  QEventLoop loop{};

  outcome::result<std::string> result = outcome::success(std::string{});

  const auto read_ = [&](auto&& read_) {
    auto tmp = file->Read(100);

    if (!tmp &&
        tmp.error() != OrbitSsh::make_error_code(OrbitSsh::Error::kEagain)) {
      char* msg;
      int length;
      libssh2_session_last_error(session->GetRawSessionPtr(), &msg, &length,
                                 false);
      LOG("ERR: %s", std::string_view{msg, static_cast<size_t>(length)});
      result = outcome::failure(tmp.error());
      loop.quit();
      return true;
    }

    if (tmp) {
      LOG("Read Bytes: %i", tmp.value().size());
      result.value().append(tmp.value());

      if (tmp.value().empty()) {
        loop.quit();
        return true;
      } else {
        QTimer::singleShot(0, [&]() { return read_(read_); });
      }
    }

    return false;
  };

  const auto read = [&]() { return read_(read_); };

  if (read()) {
    return result;
  }

  QSocketNotifier notifier{fd, QSocketNotifier::Read};
  QObject::connect(&notifier, &QSocketNotifier::activated, &loop, read);

  loop.exec();

  return result;
}

int main(int argc, char* argv[]) {
  QCoreApplication a{argc, argv};

  auto ssh_opt = GetSshInfoSync(0);
  if (!ssh_opt) {
    ERROR("Unable to get ggp ssh info");
    return -1;
  }
  GgpSshInfo ssh_info = std::move(*ssh_opt);

  OrbitSsh::Credentials credentials = {};

  credentials.host = ssh_info.host.toStdString();
  credentials.port = ssh_info.port;
  credentials.user = ssh_info.user.toStdString();
  credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
  credentials.key_path = ssh_info.key_path.toStdString();

  auto ctxWrapper = OrbitSsh::Context::Create();
  if (!ctxWrapper) {
    FATAL("Error while creating SSH context: %s", ctxWrapper.error().message());
  }
  auto& context = ctxWrapper.value();

  if (a.arguments().size() > 1 && a.arguments()[1] == "sftp") {
    LOG("Starting SFTP tunnel.");

    // Lets copy a file.
    OrbitSsh::SessionManager sessionManager{&context, credentials};
    QEventLoop loop{};

    if (OrbitSsh::shouldITryAgain(sessionManager.Initialize())) {
      const auto tick = [&]() {
        if (!OrbitSsh::shouldITryAgain(sessionManager.Initialize())) {
          loop.quit();
        }
      };

      CHECK(sessionManager.GetSocketPtr());

      QSocketNotifier notifierRead{
          static_cast<qintptr>(
              sessionManager.GetSocketPtr()->GetFileDescriptor()),
          QSocketNotifier::Read};
      QObject::connect(&notifierRead, &QSocketNotifier::activated, &a, tick);

      QSocketNotifier notifierWrite{
          static_cast<qintptr>(
              sessionManager.GetSocketPtr()->GetFileDescriptor()),
          QSocketNotifier::Write};
      QObject::connect(&notifierWrite, &QSocketNotifier::activated, &a, tick);

      loop.exec();
    }

    LOG("Connected to instance.");

    auto sftp = WaitFor(
        sessionManager.GetSocketPtr()->GetFileDescriptor(),
        [&]() { return OrbitSsh::Sftp::Init(sessionManager.GetSessionPtr()); });

    if (!sftp) {
      FATAL("Error occurred while opening sftp connection: %s",
            sftp.error().message());
    }

    LOG("SFTP channel established");

    using OrbitSsh::FxfFlags;

    auto file =
        WaitFor(sessionManager.GetSocketPtr()->GetFileDescriptor(), [&]() {
          return OrbitSsh::SftpFile::Open(
              sessionManager.GetSessionPtr(), &sftp.value(), "/tmp/test.txt",
              FxfFlags::kCreate | FxfFlags::kWrite | FxfFlags::kTruncate, 0644);
        });

    if (!file) {
      FATAL("Error occurred while opening file: %s", file.error().message());
    }

    std::string_view payload{"I was here! 42!\n"};
    auto result =
        SyncWrite(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                  &file.value(), sessionManager.GetSessionPtr(), payload);
    if (!result) {
      FATAL("Error while writing to the file: %s", result.error().message());
    }

    result = WaitFor(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                     [&]() { return file.value().Close(); });

    if (!result) {
      FATAL("Error while closing file: %s", result.error().message());
    }

    auto fileRead =
        WaitFor(sessionManager.GetSocketPtr()->GetFileDescriptor(), [&]() {
          return OrbitSsh::SftpFile::Open(sessionManager.GetSessionPtr(),
                                          &sftp.value(), "/tmp/test.txt",
                                          FxfFlags::kRead, 0644);
        });

    if (!fileRead) {
      FATAL("Error occurred while opening file: %s", fileRead.error().message());
    }

    const auto readResult =
        SyncRead(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                 &fileRead.value(), sessionManager.GetSessionPtr());

    if (!readResult) {
      FATAL("Error occurred while reading file: %s",
            readResult.error().message());
    }

    CHECK(readResult.value() == payload);
    LOG("Read string is identical to written string.");

    result = WaitFor(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                     [&]() { return fileRead.value().Close(); });

    if (!result) {
      FATAL("Error while closing file: %s", result.error().message());
    }

    result = WaitFor(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                     [&]() { return sftp.value().Shutdown(); });

    if (!result) {
      FATAL("Error while shutting down SFTP channel: %s",
            result.error().message());
    }

  } else {
    OrbitSsh::SessionManager session_manager(&context, credentials);

    // ------ session start
    outcome::result<void> session_result = outcome::success();
    do {
      session_result = session_manager.Initialize();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (OrbitSsh::shouldITryAgain(session_result));
    CHECK(session_result.has_value());
    // ------ session established

    // ------ example deployment task
    // TODO (antonrohr) here the following needs to be done:
    // 1. Check if and which version of OrbitService is already installed on
    // the gamelet.
    // 2. If the correct version is installed, this task is done.
    // 3. If the wrong version is installed:
    // 3a. Upload OrbitService debian package (using sftpfile class)
    // 3b. Check OrbitService signature
    // 3c. Install debian package

    // The following task is just a placeholder example to demonstrate
    // functionality

    OrbitSsh::Task<bool> deploy_task{
        session_manager.GetSessionPtr(), "echo \"TODO deploy task\"",
        [](std::string std_out, std::optional<bool>* result) {
          LOG("pre task std out: %s", std_out.c_str());
          if (std_out == "TODO deploy task\n") {
            *result = true;
          }
        },
        [](std::string std_out, std::optional<bool>* result) {
          LOG("pre task std err: %s", std_out.c_str());
          *result = false;
        },
        [](int exit_code, std::optional<bool>* result) -> bool {
          if (result->has_value() && *result && exit_code == 0) {
            return true;
          } else {
            return false;
          }
        }};

    outcome::result<bool> deploy_result = outcome::success(true);
    do {
      deploy_result = deploy_task.Run();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (OrbitSsh::shouldITryAgain(deploy_result));

    CHECK(deploy_result.has_value() && deploy_result.value());
    // ------ deployment done

    // Setup OrbitService Task
    OrbitSsh::Task<bool> orbit_service{
        session_manager.GetSessionPtr(), "ls /mnt/developer/OrbitService",
        [](std::string std_out, std::optional<bool>*) {
          LOG("OrbitService std out: %s", std_out.c_str());
        },
        [](std::string std_err, std::optional<bool>*) {
          LOG("OrbitService std error: %s", std_err.c_str());
        },
        [](bool exit_code, std::optional<bool>*) -> bool {
          LOG("OrbitService returned with exit_code %d", exit_code);
          return exit_code == 0;
        }};

    // Setup Tcp/Ip Tunnels
    OrbitSsh::TunnelManager tunnel_44765{session_manager.GetSessionPtr(), 44765,
                                         44765};
    OrbitSsh::TunnelManager tunnel_44766{session_manager.GetSessionPtr(), 44766,
                                         44766};

    // ------ Run OrbitService and Tunnels
    outcome::result<bool> orbit_service_result = outcome::success();
    outcome::result<void> tunnel_44765_result = outcome::success();
    outcome::result<void> tunnel_44766_result = outcome::success();

    do {
      orbit_service_result = orbit_service.Run();
      tunnel_44765_result = tunnel_44765.Tick();
      tunnel_44766_result = tunnel_44766.Tick();

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (OrbitSsh::shouldITryAgain(orbit_service_result) &&
             (tunnel_44765_result == outcome::success() ||
              OrbitSsh::shouldITryAgain(tunnel_44765_result)) &&
             (tunnel_44766_result == outcome::success() ||
              OrbitSsh::shouldITryAgain(tunnel_44766_result)));

    // ------ Orbit Service returned
    CHECK(orbit_service_result.has_value() && orbit_service_result.value());

    // ------ Close Tunnels
    outcome::result<void> close_44765_result = outcome::success();
    outcome::result<void> close_44766_result = outcome::success();
    do {
      close_44765_result = tunnel_44765.Close();
      close_44766_result = tunnel_44766.Close();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (OrbitSsh::shouldITryAgain(close_44765_result) ||
             OrbitSsh::shouldITryAgain(close_44766_result));

    // ------ Close Session
    outcome::result<void> close_session_result = outcome::success();
    do {
      close_session_result = session_manager.Close();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (OrbitSsh::shouldITryAgain(close_session_result));
  }
  return 0;
}
