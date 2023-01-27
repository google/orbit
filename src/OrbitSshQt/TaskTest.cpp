// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <QVariant>
#include <QtCore>
#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"
#include "OrbitBase/WhenAll.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SshQtTestUtils/SshTestFixture.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;

using SshTaskTest = orbit_ssh_qt_test_utils::SshTestFixture;

TEST_F(SshTaskTest, ReturnCode) {
  orbit_ssh_qt::Task task{GetSession(), "exit 42"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
  QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};

  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));
  EXPECT_EQ(started_signal.size(), 1);

  // `ServiceDeployManager` relies on the fact that `Task::Stop` triggers a `stopped()` signal even
  // when the task has already been stopped through other means. So we test that the signal has been
  // emitted at least one more time after calling `Stop`.
  const int number_of_stopped_signals_before_calling_stop = stopped_signal.size();
  constexpr Task::ExitCode kExpectedExitCode{42};
  EXPECT_THAT(WaitForWithTimeout(task.Stop()),
              YieldsResult(orbit_test_utils::HasValue(kExpectedExitCode)));
  EXPECT_GT(stopped_signal.size(), number_of_stopped_signals_before_calling_stop);

  EXPECT_THAT(finished_signal,
              testing::ElementsAre(testing::ElementsAre(QVariant{*kExpectedExitCode})));
}

TEST_F(SshTaskTest, Stdout) {
  orbit_ssh_qt::Task task{GetSession(), "echo 'Hello World'"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdOut};

  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));
  // If we haven't yet received the `finished` signal, let's wait for it.
  if (finished_signal.empty()) {
    EXPECT_TRUE(finished_signal.wait());
  }
  EXPECT_THAT(WaitForWithTimeout(task.Stop()), YieldsResult(HasNoError()));

  EXPECT_FALSE(ready_read_signal.empty());
  EXPECT_EQ(task.ReadStdOut(), "Hello World\n");
}

TEST_F(SshTaskTest, Stderr) {
  orbit_ssh_qt::Task task{GetSession(), "echo 'Hello World' >&2"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdErr};

  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));
  // If we haven't yet received the `finished` signal, let's wait for it.
  if (finished_signal.empty()) {
    EXPECT_TRUE(finished_signal.wait());
  }
  EXPECT_THAT(WaitForWithTimeout(task.Stop()), YieldsResult(HasNoError()));

  EXPECT_GE(ready_read_signal.size(), 1);
  EXPECT_EQ(task.ReadStdErr(), "Hello World\n");
}

TEST_F(SshTaskTest, Stdin) {
  orbit_ssh_qt::Task task{GetSession(), "read; echo ${REPLY}"};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdOut};

  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));

  qRegisterMetaType<size_t>("size_t");
  QSignalSpy bytes_written_signal{&task, &orbit_ssh_qt::Task::bytesWritten};
  constexpr std::string_view kInputString{"Hello World\n"};
  ASSERT_THAT(WaitForWithTimeout(task.Write(kInputString)), YieldsResult(HasNoError()));

  EXPECT_EQ(
      absl::c_accumulate(bytes_written_signal, uint64_t{0},
                         [](uint64_t lhs, const auto& signal) { return lhs + signal[0].toInt(); }),
      kInputString.size());

  std::string std_out{};
  QObject::connect(&task, &Task::readyReadStdOut, &task,
                   [&std_out, &task]() { std_out.append(task.ReadStdOut()); });

  EXPECT_TRUE(QTest::qWaitFor([&std_out, kInputString]() { return std_out == kInputString; }));

  EXPECT_THAT(WaitForWithTimeout(task.Stop()), YieldsResult(HasNoError()));
}

TEST_F(SshTaskTest, StdinMultipleWrites) {
  orbit_ssh_qt::Task task{GetSession(), "read"};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdOut};

  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));

  qRegisterMetaType<size_t>("size_t");
  QSignalSpy bytes_written_signal{&task, &orbit_ssh_qt::Task::bytesWritten};
  constexpr std::string_view kInputString{"Hello World\n"};

  // A line breaks make the read command return, so we will only send a line break in the very last
  // Write call
  constexpr std::string_view kInputStringNoLineBreak = kInputString.substr(kInputString.size() - 1);

  // We make a few writes here that will be executed asynchronously.
  std::array futures = {task.Write(kInputStringNoLineBreak), task.Write(kInputStringNoLineBreak),
                        task.Write(kInputStringNoLineBreak), task.Write(kInputStringNoLineBreak),
                        task.Write(kInputString)};

  // And here we expect all the writes to complete with no error.
  EXPECT_THAT(WaitForWithTimeout(orbit_base::WhenAll<ErrorMessageOr<void>>(futures)),
              YieldsResult(testing::Each(HasNoError())));
  EXPECT_THAT(WaitForWithTimeout(task.Stop()), YieldsResult(HasNoError()));

  const uint64_t total_bytes_written =
      absl::c_accumulate(bytes_written_signal, uint64_t{0},
                         [](uint64_t lhs, const auto& signal) { return lhs + signal[0].toInt(); });
  EXPECT_EQ(total_bytes_written, 4 * kInputStringNoLineBreak.size() + kInputString.size());
}

TEST_F(SshTaskTest, Kill) {
  orbit_ssh_qt::Task task{GetSession(), "read"};
  EXPECT_THAT(WaitForWithTimeout(task.Start()), YieldsResult(HasNoError()));

  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  constexpr Task::ExitCode kExpectedExitCode{1};
  EXPECT_THAT(WaitForWithTimeout(task.Stop()), YieldsResult(HasValue(kExpectedExitCode)));
  EXPECT_THAT(finished_signal,
              testing::ElementsAre(testing::ElementsAre(QVariant{*kExpectedExitCode})));
}

TEST_F(SshTaskTest, Execute) {
  orbit_ssh_qt::Task task{GetSession(), "echo 'Hello World'"};
  EXPECT_THAT(WaitForWithTimeout(task.Execute()), YieldsResult(HasValue(Task::ExitCode{0})));
}
}  // namespace orbit_ssh_qt