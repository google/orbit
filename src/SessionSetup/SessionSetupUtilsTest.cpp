// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QString>
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

TEST(SessionSetupUtils, ConnectionTargetFromValidInstanceID) {
  const QString instance_id = "some/i-nstanc-3/id123";
  const uint32_t pid = 1234;

  // Correct format: "pid@instance_id", where PID is a uint32_t
  QString target_string = QString("%1@%2").arg(QString::number(pid), instance_id);
  const auto result = ConnectionTarget::FromString(target_string);
  EXPECT_TRUE(result.has_value());
  if (result.has_value()) {
    EXPECT_EQ(result.value().instance_id_or_name_.toStdString(), instance_id.toStdString());
    EXPECT_EQ(result.value().process_id_, pid);
  }
}

TEST(SessionSetupUtils, ConnectionTargetFromValidInstanceName) {
  const QString instance_name = "somename-1";
  const uint32_t pid = 1234;

  // Correct format: "pid@instance_name", where PID is a uint32_t
  const QString target_string = QString("%1@%2").arg(QString::number(pid), instance_name);
  auto result = ConnectionTarget::FromString(target_string);
  EXPECT_TRUE(result.has_value());
  if (result.has_value()) {
    EXPECT_EQ(result.value().instance_id_or_name_.toStdString(), instance_name.toStdString());
    EXPECT_EQ(result.value().process_id_, pid);
  }
}

TEST(SessionSetupUtils, ErrorRaisedOnInvalidTarget) {
  // Fail-tests for multiple incorrect formats
  EXPECT_FALSE(ConnectionTarget::FromString("no/id/found").has_value());
  EXPECT_FALSE(ConnectionTarget::FromString("invalid_pid@valid/i-nstanc-3/id").has_value());
  EXPECT_FALSE(ConnectionTarget::FromString("").has_value());
  // Double "@" results in failure as it's ambiguous
  EXPECT_FALSE(ConnectionTarget::FromString("1234@invalid@i-nstanc-3/id").has_value());
}

}  // namespace orbit_session_setup