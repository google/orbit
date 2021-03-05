// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <absl/strings/str_format.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <QAbstractSocket>
#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QTemporaryFile>
#include <QTimer>
#include <chrono>
#include <filesystem>
#include <optional>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/SftpCopyToLocalOperation.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"

TEST(OrbitSshQtTests, IntegrationTest) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  auto app = QCoreApplication::instance();
  ASSERT_EQ(app->arguments().size(), 6);

  orbit_ssh::Credentials creds{
      /* .addr_and_port = */ {app->arguments()[1].toStdString(), app->arguments()[2].toInt()},
      /* .user = */ app->arguments()[3].toStdString(),
      /* .known_hosts_path = */
      std::filesystem::path{app->arguments()[4].toStdString()},
      /* .key_path = */
      std::filesystem::path{app->arguments()[5].toStdString()}};

  orbit_ssh_qt::Session session{&context.value()};
  const uint16_t port_number = 1025;
  orbit_ssh_qt::Task task{&session, absl::StrFormat("nc -l %d", port_number)};
  orbit_ssh_qt::Tunnel tunnel{&session, "127.0.0.1", port_number};
  QTcpSocket client;
  std::string data_sink;
  std::string data_sink_reverse;
  std::string greetings = "Hello World! I'm here!";
  std::string_view write_buffer{greetings};
  orbit_ssh_qt::SftpChannel sftp_channel{&session};
  orbit_ssh_qt::SftpCopyToRemoteOperation sftp_op{&session, &sftp_channel};
  QEventLoop loop{};

  QObject::connect(&client, &QTcpSocket::readyRead, &loop, [&]() {
    data_sink_reverse.append(client.readAll().toStdString());
    if (write_buffer.empty()) {
      client.close();
    }
  });

  enum class Checkpoint {
    kSessionStarted,
    kTaskStarted,
    kTaskFinished,
    kSocketConnected,
    kTunnelStarted,
    kSftpChannelStarted,
    kSftpChannelStopped,
    kSftpOperationStopped,
    kLast
  };

  uint32_t checkpoints = 0;
  const auto CheckCheckpoint = [&checkpoints](Checkpoint cp) {
    checkpoints |= 1u << static_cast<int>(cp);
  };
  const auto CheckIfAllCheckpointsAreChecked = [&checkpoints]() {
    EXPECT_EQ((1u << static_cast<int>(Checkpoint::kLast)) - 1, checkpoints);
  };

  // Session

  QObject::connect(&session, &orbit_ssh_qt::Session::errorOccurred, &loop, [&](std::error_code e) {
    loop.quit();
    FAIL() << absl::StrFormat("An error occurred while starting session: %s", e.message());
  });

  QObject::connect(&session, &orbit_ssh_qt::Session::started, &session, [&]() {
    LOG("Session connected. Starting task...");
    task.Start();
    CheckCheckpoint(Checkpoint::kSessionStarted);
  });

  // Task

  QObject::connect(&task, &orbit_ssh_qt::Task::readyReadStdOut, &loop,
                   [&]() { data_sink.append(task.ReadStdOut()); });

  QObject::connect(&task, &orbit_ssh_qt::Task::started, &loop, [&]() {
    LOG("process started. Starting tunnel...");
    task.Write("Data in reverse direction!");
    tunnel.Start();
    CheckCheckpoint(Checkpoint::kTaskStarted);
  });

  QObject::connect(&task, &orbit_ssh_qt::Task::finished, &loop, [&](int exit_code) {
    EXPECT_EQ(exit_code, 0);
    EXPECT_EQ(data_sink, "Hello World! I'm here!");
    EXPECT_EQ(data_sink_reverse, "Data in reverse direction!");
    sftp_channel.Start();
    CheckCheckpoint(Checkpoint::kTaskFinished);
  });

  // TCP Tunnel

  QObject::connect(
      &client, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
      &loop, [&]() {
        FAIL() << absl::StrFormat("TCP error occurred: %s", client.errorString().toStdString());
        loop.quit();
      });

  const auto write_bytes = [&](auto&& write_bytes) -> void {
    if (!write_buffer.empty()) {
      const auto bytes_written = client.write(write_buffer.data(), write_buffer.size());
      ASSERT_NE(bytes_written, -1) << client.errorString().toStdString();

      write_buffer = write_buffer.substr(bytes_written);

      if (!write_buffer.empty()) {
        QTimer::singleShot(0, [&]() { write_bytes(write_bytes); });
      } else if (!data_sink_reverse.empty()) {
        client.close();
      }
    } else if (!data_sink_reverse.empty()) {
      client.close();
    }
  };

  QObject::connect(&client, &QTcpSocket::connected, &loop, [&]() {
    LOG("TCP Socket connected. Writing data...");
    write_bytes(write_bytes);
    CheckCheckpoint(Checkpoint::kSocketConnected);
  });

  QObject::connect(&tunnel, &orbit_ssh_qt::Tunnel::started, &loop, [&]() {
    LOG("Tunnel opened. Connecting tcp client...");
    client.connectToHost("127.0.0.1", tunnel.GetListenPort());
    CheckCheckpoint(Checkpoint::kTunnelStarted);
  });

  // SFTP Channel
  std::optional<QTemporaryFile> temp_file;

  QObject::connect(&sftp_channel, &orbit_ssh_qt::SftpChannel::started, &loop, [&]() {
    temp_file.emplace();
    temp_file->setAutoRemove(true);
    ASSERT_TRUE(temp_file->open());
    temp_file->write("This is a test content!\nSecond line.");
    temp_file->close();

    LOG("Sftp channel opened! Starting file copy...");
    sftp_op.CopyFileToRemote(temp_file->fileName().toStdString(), "/tmp/temporary_file.txt",
                             orbit_ssh_qt::SftpCopyToRemoteOperation::FileMode::kUserWritable);
    CheckCheckpoint(Checkpoint::kSftpChannelStarted);
  });

  QObject::connect(&sftp_channel, &orbit_ssh_qt::SftpChannel::errorOccurred, &loop,
                   [&](std::error_code e) {
                     loop.quit();
                     FAIL() << absl::StrFormat("SFTP channel error occurred: %s", e.message());
                   });

  QObject::connect(&sftp_channel, &orbit_ssh_qt::SftpChannel::stopped, &loop, [&]() {
    LOG("Sftp channel closed!");
    loop.quit();
    CheckCheckpoint(Checkpoint::kSftpChannelStopped);
  });

  // SFTP Operation

  QObject::connect(&sftp_op, &orbit_ssh_qt::SftpCopyToRemoteOperation::errorOccurred, &loop,
                   [&](std::error_code e) {
                     loop.quit();
                     FAIL() << absl::StrFormat("SFTP operation error occurred: %s", e.message());
                   });

  QObject::connect(&sftp_op, &orbit_ssh_qt::SftpCopyToRemoteOperation::stopped, &loop, [&]() {
    LOG("Sftp file copy finished!");
    sftp_channel.Stop();
    CheckCheckpoint(Checkpoint::kSftpOperationStopped);
  });

  session.ConnectToServer(creds);
  LOG("connect to server");
  QTimer::singleShot(std::chrono::seconds{5}, &loop, [&]() {
    loop.quit();
    FAIL() << "Timeout occurred. The whole integration test should be done in 5 "
              "seconds. If not, probably it's stuck somewhere in the callback "
              "logic.";
  });

  loop.exec();
  CheckIfAllCheckpointsAreChecked();
}

TEST(OrbitSshQtTests, CopyToLocalTest) {
  QEventLoop loop{};

  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  auto app = QCoreApplication::instance();
  ASSERT_EQ(app->arguments().size(), 6);

  orbit_ssh::Credentials creds{
      /* .addr_and_port = */ {app->arguments()[1].toStdString(), app->arguments()[2].toInt()},
      /* .user = */ app->arguments()[3].toStdString(),
      /* .known_hosts_path = */
      std::filesystem::path{app->arguments()[4].toStdString()},
      /* .key_path = */
      std::filesystem::path{app->arguments()[5].toStdString()}};

  orbit_ssh_qt::Session session{&context.value()};

  orbit_ssh_qt::SftpChannel channel{&session};

  orbit_ssh_qt::SftpCopyToLocalOperation sftp_copy_to_local{&session, &channel};

  session.ConnectToServer(creds);
  LOG("connect to server");
  QObject::connect(&session, &orbit_ssh_qt::Session::started, &session, [&]() {
    LOG("Session connected. Starting channel...");
    channel.Start();
  });

  std::string file_name;
  QObject::connect(&channel, &orbit_ssh_qt::SftpChannel::started, &loop, [&]() {
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    file_name = temp_file.fileName().toStdString();
    temp_file.remove();

    LOG("Sftp channel opened! Starting file copy to \"%s\"...", file_name);
    sftp_copy_to_local.CopyFileToLocal("/proc/cpuinfo", file_name);
  });

  QObject::connect(&channel, &orbit_ssh_qt::SftpChannel::stopped, &loop, [&]() {
    LOG("Sftp channel closed!");
    loop.quit();
  });

  QObject::connect(&sftp_copy_to_local, &orbit_ssh_qt::SftpCopyToLocalOperation::stopped, &loop,
                   [&]() {
                     LOG("Sftp file copy finished!");
                     channel.Stop();
                   });

  QTimer::singleShot(std::chrono::seconds{5}, &loop, [&]() {
    loop.quit();
    FAIL() << "Timeout occurred. The whole integration test should be done in 5 "
              "seconds. If not, probably it's stuck somewhere in the callback "
              "logic.";
  });

  loop.exec();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  QCoreApplication app{argc, argv};
  return RUN_ALL_TESTS();
}
