// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <optional>

#include "OrbitGgp/SshInfo.h"

namespace OrbitGgp {

TEST(SshInfoTest, CreateFromJson) {
  QByteArray json;

  // Empty json
  json = QString("").toUtf8();
  EXPECT_FALSE(SshInfo::CreateFromJson(json));

  // invalid json
  json = QString("{..dfP}").toUtf8();
  EXPECT_FALSE(SshInfo::CreateFromJson(json));

  // empty object
  json = QString("{}").toUtf8();
  EXPECT_FALSE(SshInfo::CreateFromJson(json));

  // object without all necessary fields
  json = QString("{\"host\":\"0.0.0.1\"}").toUtf8();
  EXPECT_FALSE(SshInfo::CreateFromJson(json));

  // valid object

  // Pretty print test data
  // {
  //  "host": "1.1.0.1",
  //  "keyPath": "/usr/local/some/path/.ssh/id_rsa",
  //  "knownHostsPath": "/usr/local/another/path/known_hosts",
  //  "port": "11123",
  //  "user": "a username"
  // }
  json = QString(
             "{\"host\":\"1.1.0.1\",\"keyPath\":\"/usr/local/some/path/.ssh/"
             "id_rsa\",\"knownHostsPath\":\"/usr/local/another/path/"
             "known_hosts\",\"port\":\"11123\",\"user\":\"a username\"}")
             .toUtf8();
  const auto ssh_info_result = SshInfo::CreateFromJson(json);
  ASSERT_TRUE(ssh_info_result);
  const SshInfo ssh_info = std::move(ssh_info_result.value());
  EXPECT_EQ(ssh_info.host, "1.1.0.1");
  EXPECT_EQ(ssh_info.key_path,
            "/usr/local/some/path/.ssh/"
            "id_rsa");
  EXPECT_EQ(ssh_info.known_hosts_path,
            "/usr/local/another/path/"
            "known_hosts");
  EXPECT_EQ(ssh_info.port, 11123);
  EXPECT_EQ(ssh_info.user, "a username");

  // valid object - but port is formatted as an int.
  json = QString(
             "{\"host\":\"1.1.0.1\",\"keyPath\":\"/usr/local/some/path/.ssh/"
             "id_rsa\",\"knownHostsPath\":\"/usr/local/another/path/"
             "known_hosts\",\"port\":11123,\"user\":\"a username\"}")
             .toUtf8();
  // This is supposed to fail, since its expected that the port is a string
  EXPECT_FALSE(SshInfo::CreateFromJson(json));
}

}  // namespace OrbitGgp
