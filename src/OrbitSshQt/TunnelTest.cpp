// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTcpSocket>
#include <QTest>
#include <chrono>
#include <numeric>
#include <string_view>
#include <thread>

#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"
#include "SshTestFixture.h"

namespace orbit_ssh_qt {

// This test fixure inherits all the functionality from SshTestFixture and on top ensures that
// there is an echo server running on TCP port 4444 on the SSH server's machine.
class SshTunnelTest : public SshTestFixture {
 public:
  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(SshTestFixture::SetUp());  // NOLINT(cppcoreguidelines-avoid-goto)
    if (testing::Test::IsSkipped()) GTEST_SKIP();

    // socat with these options implements an echo server. It listen on incoming connections at a
    // random TCP port and replies to all incoming messages just by returning the same bytes.
    task_.emplace(GetSession(), "socat -dd tcp-listen:0 exec:'/bin/cat'");
    QSignalSpy finished_signal{&task_.value(), &orbit_ssh_qt::Task::finished};
    task_->Start();

    if (!task_->IsStarted()) {
      QSignalSpy started_signal{&task_.value(), &orbit_ssh_qt::Task::started};
      EXPECT_TRUE(started_signal.wait());
    }

    // We have to wait for socat being ready to receive connections. There is no good way to detect
    // readiness other than parsing the debug output.
    std::string socat_output{};
    ScopedConnection output_connection{
        QObject::connect(&task_.value(), &orbit_ssh_qt::Task::readyReadStdErr, &task_.value(),
                         [this, &socat_output]() { socat_output.append(task_->ReadStdErr()); })};

    ASSERT_TRUE(QTest::qWaitFor([&socat_output]() {
      // We need to make sure the first line was printed fully to ensure we can parse the port
      // number in the next step.
      return absl::StrContains(socat_output, "listening on AF=2 0.0.0.0:") &&
             absl::StrContains(socat_output, '\n');
    })) << "Socat didn't start in time. The debug output was: "
        << socat_output;

    auto lines = absl::StrSplit(socat_output, '\n');
    ASSERT_NE(lines.begin(), lines.end());
    std::string_view first_line = *lines.begin();

    constexpr std::string_view kIpAddress{"0.0.0.0:"};
    auto ip_location = first_line.find(kIpAddress);
    ASSERT_NE(ip_location, std::string_view::npos)
        << "Couldn't find the IP address in the first line: " << first_line;

    std::string_view port_as_string = first_line.substr(ip_location + kIpAddress.size());
    ASSERT_TRUE(absl::SimpleAtoi(port_as_string, &port_))
        << "Couldn't parse port number. Input was: " << port_as_string;
  }

  void TearDown() override {
    if (task_.has_value()) {
      KillRunningEchoServer();

      task_->Stop();

      if (!task_->IsStopped()) {
        QSignalSpy stopped_signal{&task_.value(), &orbit_ssh_qt::Task::stopped};
        EXPECT_TRUE(stopped_signal.wait());
      }
    }

    SshTestFixture::TearDown();
  }

  [[nodiscard]] uint16_t GetEchoServerPort() const { return static_cast<uint16_t>(port_); }

 private:
  std::optional<Task> task_;
  int port_{};

  void KillRunningEchoServer() {
    // Since we don't have PTY support implemented in `Task`, we can't just send Ctrl+C to the
    // running process. So instead we execute a kill command in a different task.

    orbit_ssh_qt::Task kill_task{GetSession(),
                                 absl::StrFormat("kill $(fuser %d/tcp)", GetEchoServerPort())};
    kill_task.Start();

    if (!kill_task.IsStarted()) {
      QSignalSpy started_signal{&kill_task, &orbit_ssh_qt::Task::started};
      EXPECT_TRUE(started_signal.wait());
    }

    if (!kill_task.IsStopped()) {
      QSignalSpy stopped_signal{&kill_task, &orbit_ssh_qt::Task::stopped};
      EXPECT_TRUE(stopped_signal.wait());
    }
  }
};

TEST_F(SshTunnelTest, StartFails) {
  constexpr int kSomeDefinitelyUnusedPort{80};
  orbit_ssh_qt::Tunnel tunnel{GetSession(), "127.0.0.1", kSomeDefinitelyUnusedPort};

  QSignalSpy tunnel_opened_signal{&tunnel, &orbit_ssh_qt::Tunnel::tunnelOpened};
  tunnel.Start();

  // Since nothing is listening on port 80, we expect the tunnel start to fail.
  if (!tunnel.IsInErrorState()) {
    qRegisterMetaType<std::error_code>("std::error_code");
    QSignalSpy error_signal{&tunnel, &orbit_ssh_qt::Tunnel::errorOccurred};
    EXPECT_TRUE(error_signal.wait());
  }
}

TEST_F(SshTunnelTest, ReadAndWrite) {
  // Once socat is up and running we will start the tunnel.
  orbit_ssh_qt::Tunnel tunnel{GetSession(), "127.0.0.1", GetEchoServerPort()};

  QSignalSpy tunnel_opened_signal{&tunnel, &orbit_ssh_qt::Tunnel::tunnelOpened};
  tunnel.Start();

  if (!tunnel.IsStarted()) {
    QSignalSpy started_signal{&tunnel, &orbit_ssh_qt::Tunnel::started};
    EXPECT_TRUE(started_signal.wait());
  }

  // The tunnel is now in `started` state, so the tunnelOpened signal should have fired.
  ASSERT_FALSE(tunnel_opened_signal.empty());
  EXPECT_EQ(tunnel_opened_signal[0][0].toInt(), tunnel.GetListenPort());

  // We now use Qt's TCP client to connect to the local listening port and see if we can reach the
  // socat echo server.
  QTcpSocket socket{};
  socket.connectToHost("127.0.0.1", tunnel.GetListenPort());
  ASSERT_TRUE(socket.waitForConnected(5000 /* ms */));

  constexpr std::string_view kInputString{"Hello World\n"};
  socket.write(kInputString.data(), kInputString.size());
  socket.waitForBytesWritten(5000 /* ms */);

  ASSERT_TRUE(QTest::qWaitFor(
      [&socket, kInputString]() { return socket.bytesAvailable() == kInputString.size(); }));

  EXPECT_EQ(socket.readAll(), QByteArray::fromRawData(kInputString.data(), kInputString.size()));

  tunnel.Stop();

  if (!tunnel.IsStopped()) {
    QSignalSpy stopped_signal{&tunnel, &orbit_ssh_qt::Tunnel::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }
}

}  // namespace orbit_ssh_qt