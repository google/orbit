// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TEST_UTILS_SSH_TEST_FIXTURE_H_
#define ORBIT_SSH_QT_TEST_UTILS_SSH_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <optional>

#include "OrbitSsh/Context.h"
#include "OrbitSshQt/Session.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SshQtTestUtils/SshSessionTest.h"

namespace orbit_ssh_qt_test_utils {

// A generic SSH test fixture that skips the tests if an SSH server is not available. It also sets
// up the session which can be accessed via `GetSession`.
class SshTestFixture : public SshSessionTest {
 public:
  explicit SshTestFixture(
      std::string environment_variable = std::string{kSimpleSshServerEnvironmentVariableName})
      : SshSessionTest(std::move(environment_variable)) {}

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

    ASSERT_THAT(
        orbit_qt_test_utils::WaitForWithTimeout(session_->ConnectToServer(GetCredentials())),
        orbit_qt_test_utils::YieldsResult(orbit_test_utils::HasNoError()));
  }

  void TearDown() override {
    if (session_.has_value()) {
      EXPECT_THAT(orbit_qt_test_utils::WaitForWithTimeout(session_->Disconnect()),
                  orbit_qt_test_utils::YieldsResult(orbit_test_utils::HasNoError()));
    }

    SshSessionTest::TearDown();
  }

  [[nodiscard]] orbit_ssh_qt::Session* GetSession() { return &session_.value(); }
  [[nodiscard]] const orbit_ssh_qt::Session* GetSession() const { return &session_.value(); }

 private:
  std::optional<orbit_ssh::Context> context_;
  std::optional<orbit_ssh_qt::Session> session_;
};
}  // namespace orbit_ssh_qt_test_utils

#endif  // ORBIT_SSH_QT_TEST_UTILS_SSH_TEST_FIXTURE_H_
