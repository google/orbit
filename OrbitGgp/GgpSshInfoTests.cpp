// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <optional>

#include "OrbitGgp/GgpSshInfo.h"

namespace OrbitGgp {

TEST(GgpSshInfoTest, CreateFromJson) {
  QByteArray json;
  std::optional<GgpSshInfo> test_info_opt;

  // Empty json
  json = QString("").toUtf8();
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  EXPECT_FALSE(test_info_opt);

  // invalid json
  json = QString("{..dfP}").toUtf8();
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  EXPECT_FALSE(test_info_opt);

  // empty object
  json = QString("{}").toUtf8();
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  EXPECT_FALSE(test_info_opt);

  // object without all necessary fields
  json = QString("{\"host\":\"0.0.0.1\"}").toUtf8();
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  EXPECT_FALSE(test_info_opt);

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
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  ASSERT_TRUE(test_info_opt);
  EXPECT_EQ(test_info_opt->host, "1.1.0.1");
  EXPECT_EQ(test_info_opt->key_path,
            "/usr/local/some/path/.ssh/"
            "id_rsa");
  EXPECT_EQ(test_info_opt->known_hosts_path,
            "/usr/local/another/path/"
            "known_hosts");
  EXPECT_EQ(test_info_opt->port, 11123);
  EXPECT_EQ(test_info_opt->user, "a username");

  // valid object - but port is formatted as an int.
  json = QString(
             "{\"host\":\"1.1.0.1\",\"keyPath\":\"/usr/local/some/path/.ssh/"
             "id_rsa\",\"knownHostsPath\":\"/usr/local/another/path/"
             "known_hosts\",\"port\":11123,\"user\":\"a username\"}")
             .toUtf8();
  test_info_opt = GgpSshInfo::CreateFromJson(json);
  // This is supposed to fail, since its expected that the port is a string
  EXPECT_FALSE(test_info_opt);
}

}  // namespace OrbitGgp
