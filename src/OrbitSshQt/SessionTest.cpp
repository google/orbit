// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "SshSessionTest.h"

namespace orbit_ssh_qt {
TEST_F(SshSessionTest, Connect) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_TRUE(context);

  orbit_ssh_qt::Session session{&context.value()};

  session.ConnectToServer(GetCredentials());

  if (!session.IsStarted()) {
    QSignalSpy started_signal{&session, &orbit_ssh_qt::Session::started};
    EXPECT_TRUE(started_signal.wait());
  }

  if (session.Disconnect() != orbit_ssh_qt::Session::DisconnectResult::kDisconnectedSuccessfully) {
    QSignalSpy stopped_signal{&session, &orbit_ssh_qt::Session::stopped};
    EXPECT_TRUE(stopped_signal.wait());
  }
}
}  // namespace orbit_ssh_qt