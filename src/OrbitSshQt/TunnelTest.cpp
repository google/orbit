// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <QByteArray>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QSignalSpy>
#include <QTcpSocket>
#include <QTest>
#include <QVariant>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <variant>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Task.h"
#include "OrbitSshQt/Tunnel.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SshQtTestUtils/KillProcessListeningOnTcpPort.h"
#include "SshQtTestUtils/ParsePortNumberFromSocatOutput.h"
#include "SshQtTestUtils/SshTestFixture.h"
#include "TestUtils/TestUtils.h"

Q_DECLARE_METATYPE(std::error_code);

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasErrorWithMessage;
using orbit_test_utils::HasNoError;

// This test fixure inherits all the functionality from SshTestFixture and on top ensures that
// there is an echo server running on TCP port 4444 on the SSH server's machine.
class SshTunnelTest : public orbit_ssh_qt_test_utils::SshTestFixture {
 public:
  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(SshTestFixture::SetUp());  // NOLINT(cppcoreguidelines-avoid-goto)
    if (testing::Test::IsSkipped()) GTEST_SKIP();

    // socat with these options implements an echo server. It listen on incoming connections at a
    // random TCP port and replies to all incoming messages just by returning the same bytes.
    task_.emplace(GetSession(), "socat -dd tcp-listen:0,fork exec:'/bin/cat'");

    // We have to wait for socat being ready to receive connections. There is no good way to detect
    // readiness other than parsing the debug output.
    std::string socat_output{};
    ScopedConnection output_connection{
        QObject::connect(&task_.value(), &orbit_ssh_qt::Task::readyReadStdErr, &task_.value(),
                         [this, &socat_output]() { socat_output.append(task_->ReadStdErr()); })};

    ASSERT_THAT(orbit_qt_test_utils::WaitForWithTimeout(task_->Start()),
                orbit_qt_test_utils::YieldsResult(orbit_test_utils::HasNoError()));

    std::optional<ErrorMessageOr<int>> port_or_error{};
    std::ignore = QTest::qWaitFor([&socat_output, &port_or_error]() {
      port_or_error = orbit_ssh_qt_test_utils::ParsePortNumberFromSocatOutput(socat_output);
      return port_or_error.has_value();
    });
    ASSERT_TRUE(port_or_error.has_value()) << "Parsing port number from socat output timed out.";
    ASSERT_THAT(*port_or_error, HasNoError())
        << "Error occurred while parsing the port number from the socat output: "
        << port_or_error->error().message();

    port_ = port_or_error->value();
  }

  void TearDown() override {
    if (task_.has_value()) {
      // Since our Task implementation doesn't allocate a PTY, we can't just send a Ctrl+C to the
      // running socat process. Instead we call `kill` through another task running in the same
      // session.
      ASSERT_THAT(
          orbit_ssh_qt_test_utils::KillProcessListeningOnTcpPort(GetSession(), GetEchoServerPort()),
          HasNoError());

      EXPECT_THAT(orbit_qt_test_utils::WaitForWithTimeout(task_->Stop()),
                  orbit_qt_test_utils::YieldsResult(orbit_test_utils::HasNoError()));
    }

    SshTestFixture::TearDown();
  }

  [[nodiscard]] uint16_t GetEchoServerPort() const { return static_cast<uint16_t>(port_); }

 private:
  std::optional<Task> task_;
  int port_{};
};

TEST_F(SshTunnelTest, StartFails) {
  constexpr int kSomeDefinitelyUnusedPort{80};
  orbit_ssh_qt::Tunnel tunnel{GetSession(), "127.0.0.1", kSomeDefinitelyUnusedPort};

  QSignalSpy tunnel_opened_signal{&tunnel, &orbit_ssh_qt::Tunnel::tunnelOpened};
  qRegisterMetaType<std::error_code>("std::error_code");
  QSignalSpy error_signal{&tunnel, &orbit_ssh_qt::Tunnel::errorOccurred};

  EXPECT_THAT(WaitForWithTimeout(tunnel.Start()),
              YieldsResult(HasErrorWithMessage("Channel failure")));
  EXPECT_THAT(error_signal, testing::SizeIs(1));
}

TEST_F(SshTunnelTest, ReadAndWrite) {
  // Once socat is up and running we will start the tunnel.
  orbit_ssh_qt::Tunnel tunnel{GetSession(), "127.0.0.1", GetEchoServerPort()};

  QSignalSpy tunnel_opened_signal{&tunnel, &orbit_ssh_qt::Tunnel::tunnelOpened};
  std::ignore = tunnel.Start();

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

  std::ignore = tunnel.Stop();

  if (!tunnel.IsStopped()) {
    QSignalSpy stopped_signal{&tunnel, &orbit_ssh_qt::Tunnel::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }
}

}  // namespace orbit_ssh_qt
