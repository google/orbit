// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "ClientData/ProcessData.h"
#include "GrpcProtos/process.pb.h"
#include "SessionSetup/SessionSetupUtils.h"

namespace orbit_session_setup {

// Tests below need to be adjusted if the name changes, they are conveniently set up
// for process names that are exactly at the length limit
static_assert(kMaxProcessNameLength == 15);

const uint32_t kPid = 100;
const char* kFullProcessName = "ok_process_name_long";
const char* kShortProcessName = "ok_process_name";
const char* kProcessPath = "/path/to/ok_process_name_long";

std::vector<orbit_grpc_protos::ProcessInfo> SetupTestProcessList() {
  using orbit_grpc_protos::ProcessInfo;

  ProcessInfo expected_target_process;
  expected_target_process.set_pid(kPid);
  expected_target_process.set_name(kShortProcessName);
  expected_target_process.set_full_path(kProcessPath);

  ProcessInfo lower_pid_process1;
  lower_pid_process1.set_pid(kPid - 1);
  lower_pid_process1.set_name(kShortProcessName);
  lower_pid_process1.set_full_path(kProcessPath);

  ProcessInfo lower_pid_process2;
  lower_pid_process2.set_pid(kPid - 2);
  lower_pid_process2.set_name(kShortProcessName);
  lower_pid_process2.set_full_path(kProcessPath);

  ProcessInfo different_process1;
  different_process1.set_pid(kPid + 1);
  different_process1.set_name("some_other_process");
  different_process1.set_full_path("/path/to/some_other_process");

  ProcessInfo different_process2;
  different_process1.set_pid(kPid + 2);
  different_process1.set_name("some_other_process");
  different_process1.set_full_path("/path/to/some_other_process");

  // Try to add different combinations of PID sorting order and different process names before and
  // after the expected target process
  return {different_process1, lower_pid_process1, expected_target_process, different_process2,
          lower_pid_process2};
}

TEST(SessionSetupUtils, TryToFindProcessDataFindsProcessByShortName) {
  std::vector<orbit_grpc_protos::ProcessInfo> processes = SetupTestProcessList();

  EXPECT_EQ(kPid, TryToFindProcessData(processes, kShortProcessName)->pid());
}

TEST(SessionSetupUtils, TryToFindProcessDataFindsProcessByLongName) {
  std::vector<orbit_grpc_protos::ProcessInfo> processes = SetupTestProcessList();

  EXPECT_EQ(kPid, TryToFindProcessData(processes, kFullProcessName)->pid());
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