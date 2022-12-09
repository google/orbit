// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SSH_TEST_FIXTURE_H_
#define ORBIT_SSH_QT_SSH_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <optional>

#include "OrbitSsh/Context.h"
#include "OrbitSshQt/Session.h"
#include "SshSessionTest.h"

namespace orbit_ssh_qt {

// A generic SSH test fixture that skips the tests if an SSH server is not available. It also sets
// up the session which can be accessed via `GetSession`.
class SshTestFixture : public SshSessionTest {
 public:
  // We can't use the constructor here because neither GTEST_SKIP, nor ASSERT_THAT are supported in
  // constructors.
  void SetUp() override {
    // Neither asserts, nor skips propagate, so we have to handle them manually.
    ASSERT_NO_FATAL_FAILURE(SshSessionTest::SetUp());  // NOLINT(cppcoreguidelines-avoid-goto)
    if (testing::Test::IsSkipped()) GTEST_SKIP();

    auto context = orbit_ssh::Context::Create();
    ASSERT_TRUE(context.has_value());
    context_ = std::move(context).value();

    session_.emplace(&context_.value());

    session_->ConnectToServer(GetCredentials());

    if (!session_->IsStarted()) {
      QSignalSpy started_signal{&session_.value(), &orbit_ssh_qt::Session::started};
      EXPECT_TRUE(started_signal.wait());
    }
  }

  void TearDown() override {
    if (session_.has_value() &&
        session_->Disconnect() !=
            orbit_ssh_qt::Session::DisconnectResult::kDisconnectedSuccessfully) {
      QSignalSpy stopped_signal{&session_.value(), &orbit_ssh_qt::Session::stopped};
      EXPECT_TRUE(stopped_signal.wait());
    }

    SshSessionTest::TearDown();
  }

  [[nodiscard]] Session* GetSession() { return &session_.value(); }
  [[nodiscard]] const Session* GetSession() const { return &session_.value(); }

 private:
  std::optional<orbit_ssh::Context> context_;
  std::optional<Session> session_;
};
}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SSH_TEST_FIXTURE_H_
