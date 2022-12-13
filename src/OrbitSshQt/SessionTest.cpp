// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "QtTestUtils/WaitFor.h"
#include "SshSessionTest.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitFor;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasNoError;
using testing::IsEmpty;
using testing::Not;

TEST_F(SshSessionTest, Connect) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};
  QSignalSpy started_signal{&session, &Session::started};
  QSignalSpy stopped_signal{&session, &Session::stopped};

  EXPECT_THAT(WaitFor(session.ConnectToServer(GetCredentials())), YieldsResult(HasNoError()));
  EXPECT_THAT(started_signal, Not(IsEmpty()));
  EXPECT_THAT(stopped_signal, IsEmpty());
  started_signal.clear();

  EXPECT_THAT(WaitFor(session.Disconnect()), YieldsResult(HasNoError()));
  EXPECT_THAT(started_signal, IsEmpty());
  EXPECT_THAT(stopped_signal, Not(IsEmpty()));
}
}  // namespace orbit_ssh_qt