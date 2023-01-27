// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <algorithm>
#include <filesystem>
#include <variant>
#include <vector>

#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SshQtTestUtils/SshSessionTest.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using testing::IsEmpty;
using testing::Not;

using SshSessionTest = orbit_ssh_qt_test_utils::SshSessionTest;

TEST_F(SshSessionTest, Connect) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};
  QSignalSpy started_signal{&session, &Session::started};
  QSignalSpy stopped_signal{&session, &Session::stopped};

  EXPECT_THAT(WaitForWithTimeout(session.ConnectToServer(GetCredentials())),
              YieldsResult(HasNoError()));
  EXPECT_THAT(started_signal, Not(IsEmpty()));
  EXPECT_THAT(stopped_signal, IsEmpty());
  started_signal.clear();

  EXPECT_THAT(WaitForWithTimeout(session.Disconnect()), YieldsResult(HasNoError()));
  EXPECT_THAT(started_signal, IsEmpty());
  EXPECT_THAT(stopped_signal, Not(IsEmpty()));
}

TEST_F(SshSessionTest, ConnectFailsWithNonExistingKeyFile) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};
  QSignalSpy started_signal{&session, &Session::started};
  QSignalSpy stopped_signal{&session, &Session::stopped};

  orbit_ssh::Credentials credentials = GetCredentials();
  credentials.key_paths.front() = "/does/not/exist";
  EXPECT_THAT(WaitForWithTimeout(session.ConnectToServer(credentials)), YieldsResult(HasError()));
}

TEST_F(SshSessionTest, ConnectFailsWithValidButWrongKeyFile) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};

  orbit_ssh::Credentials credentials = GetCredentials();
  credentials.key_paths.front() = orbit_test::GetTestdataDir() / "random_identity";
  EXPECT_THAT(orbit_base::FileOrDirectoryExists(credentials.key_paths.front()),
              orbit_test_utils::HasValue(true));

  EXPECT_THAT(WaitForWithTimeout(session.ConnectToServer(credentials)), YieldsResult(HasError()));
}

TEST_F(SshSessionTest, ConnectSucceedsOnSecondKeyFile) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};

  orbit_ssh::Credentials credentials = GetCredentials();
  credentials.key_paths.insert(credentials.key_paths.begin(),
                               orbit_test::GetTestdataDir() / "random_identity");
  EXPECT_THAT(orbit_base::FileOrDirectoryExists(credentials.key_paths.front()),
              orbit_test_utils::HasValue(true));

  EXPECT_THAT(WaitForWithTimeout(session.ConnectToServer(credentials)), YieldsResult(HasNoError()));
  EXPECT_THAT(WaitForWithTimeout(session.Disconnect()), YieldsResult(HasNoError()));
}

TEST_F(SshSessionTest, ConnectSuceedsOnSecondKeyFileFirstInvalid) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};

  orbit_ssh::Credentials credentials = GetCredentials();
  credentials.key_paths.insert(credentials.key_paths.begin(), "/does/not/exist");
  EXPECT_THAT(WaitForWithTimeout(session.ConnectToServer(credentials)), YieldsResult(HasNoError()));
  EXPECT_THAT(WaitForWithTimeout(session.Disconnect()), YieldsResult(HasNoError()));
}
}  // namespace orbit_ssh_qt