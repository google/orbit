// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QString>
#include <filesystem>

#include "GrpcProtos/process.pb.h"
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

const uint32_t kPid = 100;
const char* kProcessName = "process_name";
const char* kProcessPath = "/path/to/process_name";

std::vector<orbit_grpc_protos::ProcessInfo> SetupTestProcessList() {
  using orbit_grpc_protos::ProcessInfo;

  auto expected_target_process = ProcessInfo();
  expected_target_process.set_pid(kPid);
  expected_target_process.set_name(kProcessName);
  expected_target_process.set_full_path(kProcessPath);

  auto lower_pid_process = ProcessInfo();
  lower_pid_process.set_pid(kPid - 1);
  lower_pid_process.set_name(kProcessName);
  lower_pid_process.set_full_path(kProcessPath);

  auto different_process = ProcessInfo();
  different_process.set_pid(kPid + 1);
  different_process.set_name("some_other_process");
  different_process.set_full_path("/path/to/some_other_process");

  return {different_process, lower_pid_process, expected_target_process, different_process,
          lower_pid_process};
}

TEST(SessionSetupUtils, TryToFindProcessDataFindsProcessByName) {
  std::vector<orbit_grpc_protos::ProcessInfo> processes = SetupTestProcessList();

  EXPECT_EQ(kPid, TryToFindProcessData(processes, kProcessName)->pid());
}

TEST(SessionSetupUtils, TryToFindProcessDataFindsProcessByPath) {
  std::vector<orbit_grpc_protos::ProcessInfo> processes = SetupTestProcessList();

  EXPECT_EQ(kPid, TryToFindProcessData(processes, kProcessPath)->pid());
}

TEST(SessionSetupUtils, TryToFindProcessDataReturnsNullOnFailure) {
  std::vector<orbit_grpc_protos::ProcessInfo> processes = SetupTestProcessList();

  EXPECT_EQ(nullptr, TryToFindProcessData(processes, "nonexisting_process"));
}

}  // namespace orbit_session_setup