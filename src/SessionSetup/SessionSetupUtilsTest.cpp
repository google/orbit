// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>

#include "OrbitGgp/SshInfo.h"
#include "OrbitSsh/Credentials.h"
#include "SessionSetup/SessionSetupUtils.h"

namespace orbit_session_setup {

TEST(SessionSetupUtils, CredentialsFromSshInfoWorksCorrectly) {
  orbit_ggp::SshInfo info;
  info.host = "127.0.0.1";
  info.key_path = "invalid/key/path";
  info.known_hosts_path = "invalid/known/hosts/path";
  info.port = 123;
  info.user = "some_user";

  orbit_ssh::Credentials credentials = CredentialsFromSshInfo(info);

  EXPECT_EQ(info.host.toStdString(), credentials.addr_and_port.addr);
  EXPECT_EQ(info.port, credentials.addr_and_port.port);
  EXPECT_EQ(std::filesystem::path{info.key_path.toStdString()}, credentials.key_path);
  EXPECT_EQ(std::filesystem::path{info.known_hosts_path.toStdString()},
            credentials.known_hosts_path);
  EXPECT_EQ(info.user.toStdString(), credentials.user);
}

}  // namespace orbit_session_setup