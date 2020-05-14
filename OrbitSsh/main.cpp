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
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Sftp.h"
#include "OrbitSsh/SftpFile.h"
#include "OrbitSsh/SshManager.h"

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
 * or an error occured. The implementation uses QEventLoop and QSocketNotifier
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

    if (result || result.error() !=
                      OrbitSsh::make_error_code(OrbitSsh::SftpError::kEagain)) {
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
 * or an error occured. The implementation uses QEventLoop and QSocketNotifier
 * instead of a bare select() to show how integration works with a event loop.
 **/
static outcome::result<void> SyncWrite(qintptr fd, OrbitSsh::SftpFile* file,
                                       OrbitSsh::Session* session,
                                       std::string_view data) {
  QEventLoop loop{};

  outcome::result<void> result = outcome::success();

  const auto write = [&]() {
    auto tmp = file->Write(data);

    if (!tmp && tmp.error() !=
                    OrbitSsh::make_error_code(OrbitSsh::SftpError::kEagain)) {
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
 * error occured. The implementation uses QEventLoop and QSocketNotifier instead
 * of a bare select() to show how integration works with a event loop.
 **/
static outcome::result<std::string> SyncRead(qintptr fd,
                                             OrbitSsh::SftpFile* file,
                                             OrbitSsh::Session* session) {
  QEventLoop loop{};

  outcome::result<std::string> result = outcome::success(std::string{});

  const auto read_ = [&](auto&& read_) {
    auto tmp = file->Read(100);

    if (!tmp && tmp.error() !=
                    OrbitSsh::make_error_code(OrbitSsh::SftpError::kEagain)) {
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

  if (a.arguments().size() > 1 && a.arguments()[1] == "sftp") {
    LOG("Starting SFTP tunnel.");

    // Lets copy a file.
    OrbitSsh::SessionManager sessionManager{credentials};
    QEventLoop loop{};

    if (sessionManager.Tick() !=
        OrbitSsh::SessionManager::State::kAuthenticated) {
      const auto tick = [&]() {
        if (sessionManager.Tick() ==
            OrbitSsh::SessionManager::State::kAuthenticated) {
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
      FATAL("Error occured while opening sftp connection: %s",
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
      FATAL("Error occured while opening file: %s", file.error().message());
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
      FATAL("Error occured while opening file: %s", fileRead.error().message());
    }

    const auto readResult =
        SyncRead(sessionManager.GetSocketPtr()->GetFileDescriptor(),
                 &fileRead.value(), sessionManager.GetSessionPtr());

    if (!readResult) {
      FATAL("Error occured while reading file: %s",
            readResult.error().message());
    }

    CHECK(readResult.value() == payload);
    LOG("Read string is identical to written string.");

  } else {
    std::queue<OrbitSsh::SshManager::Task> pre_tasks;
    pre_tasks.emplace(OrbitSsh::SshManager::Task{
        "ls -al /mnt/developer",
        [](std::string output) { LOG("Pre task output: %s", output.c_str()); },
        [](int exit_code) {
          LOG("Pre task finished with exit code %d", exit_code);
        }});

    OrbitSsh::SshManager::Task main_task = {
        "ls -al /mnt/developer/",
        [](std::string output) { LOG("Main task output: %s", output.c_str()); },
        [](int exit_code) { LOG("Man task exit code: %d", exit_code); }};

    OrbitSsh::SshManager ssh_handler(credentials, pre_tasks, main_task,
                                     {44766, 44755});

    for (int i = 0; i < 5000; i++) {
      ssh_handler.Tick();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    OrbitSsh::ResultType close_result;
    do {
      close_result = ssh_handler.Close();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (close_result == OrbitSsh::ResultType::kAgain);
  }
  return 0;
}
