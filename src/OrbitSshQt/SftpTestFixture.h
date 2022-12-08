// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SFTP_TEST_FIXTURE_H_
#define ORBIT_SSH_QT_SFTP_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <optional>

#include "OrbitSshQt/SftpChannel.h"
#include "SshTestFixture.h"

namespace orbit_ssh_qt {

// A generic SSH test fixture that skips the tests if an SSH server is not available. It also sets
// up the session which can be accessed via `GetSession`.
class SftpTestFixture : public SshTestFixture {
 public:
  // We can't use the constructor here because neither GTEST_SKIP, nor ASSERT_THAT are supported in
  // constructors.
  void SetUp() override {
    // Neither asserts, nor skips propagate, so we have to handle them manually.
    ASSERT_NO_FATAL_FAILURE(SshTestFixture::SetUp());  // NOLINT(cppcoreguidelines-avoid-goto)
    if (testing::Test::IsSkipped()) GTEST_SKIP();

    channel_.emplace(GetSession());
    channel_->Start();

    if (!channel_->IsStarted()) {
      QSignalSpy started_signal{&channel_.value(), &orbit_ssh_qt::SftpChannel::started};
      ASSERT_TRUE(started_signal.wait());
    }
  }

  void TearDown() override {
    if (channel_.has_value()) {
      channel_->Stop();

      if (!channel_->IsStopped()) {
        QSignalSpy stopped_signal{&channel_.value(), &orbit_ssh_qt::SftpChannel::stopped};
        EXPECT_TRUE(stopped_signal.wait());
      }
    }
    SshTestFixture::TearDown();
  }

  [[nodiscard]] SftpChannel* GetSftpChannel() { return &channel_.value(); }
  [[nodiscard]] const SftpChannel* GetSftpChannel() const { return &channel_.value(); }

 private:
  std::optional<SftpChannel> channel_;
};
}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SFTP_TEST_FIXTURE_H_
