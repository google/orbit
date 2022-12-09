// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>
#include <numeric>

#include "OrbitSshQt/Task.h"
#include "SshTestFixture.h"

namespace orbit_ssh_qt {

using SshTaskTest = SshTestFixture;

TEST_F(SshTaskTest, ReturnCode) {
  orbit_ssh_qt::Task task{GetSession(), "exit 42"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  task.Start();

  if (!task.IsStarted()) {
    QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
    EXPECT_TRUE(started_signal.wait());
  }

  if (!task.IsStopped()) {
    QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }

  ASSERT_EQ(finished_signal.size(), 1);
  EXPECT_EQ(finished_signal[0][0].toInt(), 42);
}

TEST_F(SshTaskTest, Stdout) {
  orbit_ssh_qt::Task task{GetSession(), "echo 'Hello World'"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdOut};
  task.Start();

  if (!task.IsStarted()) {
    QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
    EXPECT_TRUE(started_signal.wait());
  }

  if (!task.IsStopped()) {
    QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }

  EXPECT_FALSE(ready_read_signal.empty());
  EXPECT_EQ(task.ReadStdOut(), "Hello World\n");
}

TEST_F(SshTaskTest, Stderr) {
  orbit_ssh_qt::Task task{GetSession(), "echo 'Hello World' >&2"};
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdErr};
  task.Start();

  if (!task.IsStarted()) {
    QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
    EXPECT_TRUE(started_signal.wait());
  }

  if (!task.IsStopped()) {
    QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }

  EXPECT_FALSE(ready_read_signal.empty());
  EXPECT_EQ(task.ReadStdErr(), "Hello World\n");
}

TEST_F(SshTaskTest, Stdin) {
  orbit_ssh_qt::Task task{GetSession(), "read; echo ${REPLY}"};
  QSignalSpy ready_read_signal{&task, &orbit_ssh_qt::Task::readyReadStdOut};
  task.Start();

  if (!task.IsStarted()) {
    QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
    EXPECT_TRUE(started_signal.wait());
  }

  qRegisterMetaType<size_t>("size_t");
  QSignalSpy bytes_written_signal{&task, &orbit_ssh_qt::Task::bytesWritten};
  constexpr std::string_view kInputString{"Hello World\n"};
  task.Write(kInputString);

  // Here we wait until all the bytes are written. Note that there is no guarantee on how many
  // separate internal writes that takes.
  ASSERT_TRUE(QTest::qWaitFor([&bytes_written_signal, kInputString]() {
    const auto add_bytes_written_per_signal = [](size_t sum,
                                                 const QList<QVariant>& signal_arguments) {
      return sum + static_cast<size_t>(signal_arguments.first().toInt());
    };
    const size_t bytes_written =
        absl::c_accumulate(bytes_written_signal, size_t{0}, add_bytes_written_per_signal);
    return bytes_written == kInputString.size();
  }));

  if (!task.IsStopped()) {
    QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }

  EXPECT_FALSE(ready_read_signal.empty());
  EXPECT_EQ(task.ReadStdOut(), kInputString);
}

TEST_F(SshTaskTest, Kill) {
  orbit_ssh_qt::Task task{GetSession(), "read"};
  task.Start();

  if (!task.IsStarted()) {
    QSignalSpy started_signal{&task, &orbit_ssh_qt::Task::started};
    EXPECT_TRUE(started_signal.wait());
  }

  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  task.Stop();

  if (!task.IsStopped()) {
    QSignalSpy stopped_signal{&task, &orbit_ssh_qt::Task::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }

  EXPECT_FALSE(finished_signal.empty());
  EXPECT_EQ(finished_signal[0][0].toInt(), 1);
}
}  // namespace orbit_ssh_qt