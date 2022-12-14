// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <memory>

#include "OrbitSshQt/SftpChannel.h"
#include "SshQtTestUtils/SshTestFixture.h"

namespace orbit_ssh_qt {

using SftpChannelTest = orbit_ssh_qt_test_utils::SshTestFixture;

TEST_F(SftpChannelTest, Open) {
  orbit_ssh_qt::SftpChannel channel{GetSession()};
  channel.Start();

  if (!channel.IsStarted()) {
    QSignalSpy started_signal{&channel, &orbit_ssh_qt::SftpChannel::started};
    EXPECT_TRUE(started_signal.wait());
  }

  channel.Stop();

  if (!channel.IsStopped()) {
    QSignalSpy stopped_signal{&channel, &orbit_ssh_qt::SftpChannel::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }
}
}  // namespace orbit_ssh_qt