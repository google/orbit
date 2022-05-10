// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ClientServices/LaunchedProcess.h"
#include "ClientServices/ProcessClient.h"
#include "MockProcessClient.h"
#include "OrbitBase/Logging.h"
#include "TestUtils/TestUtils.h"

namespace orbit_client_services {

using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ProcessToLaunch;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;
using ::testing::Return;

TEST(LaunchedProcess, LaunchProcess) {
  MockProcessClient mock_process_client;
  // Setup a process to be launched and executed straight away, not spinning at entry point.
  orbit_grpc_protos::ProcessToLaunch process_to_launch;
  process_to_launch.set_spin_at_entry_point(false);

  // The "MockProcessClient" returns a valid ProcessInfo when its "LaunchProcess" method is called.
  ON_CALL(mock_process_client, LaunchProcess).WillByDefault(Return(ProcessInfo()));
  EXPECT_CALL(mock_process_client, LaunchProcess).Times(1);

  // "LaunchedProcess::LaunchProcess" function will invoke "MockProcessClient::LaunchProcess".
  ErrorMessageOr<LaunchedProcess> launch_result =
      LaunchedProcess::LaunchProcess(process_to_launch, &mock_process_client);

  // Make sure the returned "LaunchedProcess" is valid and is not set as "spinning on entry point".
  ASSERT_THAT(launch_result, HasValue());
  LaunchedProcess& launched_process = launch_result.value();
  ASSERT_FALSE(launched_process.IsProcessSpinningAtEntryPoint());
  ASSERT_FALSE(launched_process.IsProcessSuspendedAtEntryPoint());
  ASSERT_TRUE(launched_process.IsProcessExecutingOrExited());

  // Calling "SuspendProcessSpinningAtEntryPoint" should abort the program since the process was not
  // launched spinning at entry point.
  EXPECT_DEATH((void)launched_process.SuspendProcessSpinningAtEntryPoint(&mock_process_client),
               "Check failed");
}

TEST(LaunchedProcess, LaunchProcessSpinningAtEntryPoint) {
  MockProcessClient mock_process_client;
  // Setup a process to be launched spinning at entry point.
  orbit_grpc_protos::ProcessToLaunch process_to_launch;
  process_to_launch.set_spin_at_entry_point(true);

  // The "MockProcessClient" returns a valid ProcessInfo when its "LaunchProcess" method is called.
  ON_CALL(mock_process_client, LaunchProcess).WillByDefault(Return(ProcessInfo()));
  EXPECT_CALL(mock_process_client, LaunchProcess).Times(1);

  // "LaunchedProcess::LaunchProcess" function will invoke "MockProcessClient::LaunchProcess".
  ErrorMessageOr<LaunchedProcess> launch_result =
      LaunchedProcess::LaunchProcess(process_to_launch, &mock_process_client);

  // Make sure the returned "LaunchedProcess" is valid and is set as "spinning on entry point".
  ASSERT_THAT(launch_result, HasValue());
  LaunchedProcess& launched_process = launch_result.value();
  ASSERT_TRUE(launched_process.IsProcessSpinningAtEntryPoint());
  ASSERT_FALSE(launched_process.IsProcessSuspendedAtEntryPoint());
  ASSERT_FALSE(launched_process.IsProcessExecutingOrExited());

  // Suspend the process spinning at entry point, making the launched process change state.
  ON_CALL(mock_process_client, SuspendProcessSpinningAtEntryPoint)
      .WillByDefault(Return(outcome::success()));
  EXPECT_CALL(mock_process_client, SuspendProcessSpinningAtEntryPoint).Times(1);
  auto suspend_result = launched_process.SuspendProcessSpinningAtEntryPoint(&mock_process_client);
  ASSERT_THAT(suspend_result, HasValue());
  ASSERT_FALSE(launched_process.IsProcessSpinningAtEntryPoint());
  ASSERT_TRUE(launched_process.IsProcessSuspendedAtEntryPoint());
  ASSERT_FALSE(launched_process.IsProcessExecutingOrExited());

  // Resume the process suspended at entry point, making the launched process change state.
  ON_CALL(mock_process_client, ResumeProcessSuspendedAtEntryPoint)
      .WillByDefault(Return(outcome::success()));
  EXPECT_CALL(mock_process_client, ResumeProcessSuspendedAtEntryPoint).Times(1);
  auto resume_result = launched_process.ResumeProcessSuspendedAtEntryPoint(&mock_process_client);
  ASSERT_THAT(resume_result, HasValue());
  ASSERT_FALSE(launched_process.IsProcessSpinningAtEntryPoint());
  ASSERT_FALSE(launched_process.IsProcessSuspendedAtEntryPoint());
  ASSERT_TRUE(launched_process.IsProcessExecutingOrExited());
}

}  // namespace orbit_client_services
