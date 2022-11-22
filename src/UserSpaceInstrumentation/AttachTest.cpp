// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <set>
#include <vector>

#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Result.h"
#include "TestProcess.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

using orbit_test_utils::HasError;

TEST(AttachTest, AttachAndStopProcess) {
  TestProcess test_process;
  const pid_t pid = test_process.pid();

  EXPECT_THAT(AttachAndStopProcess(-1), HasError("There is no process with pid"));

  const auto result = AttachAndStopProcess(pid);
  ASSERT_FALSE(result.has_error()) << result.error().message();

  // TestProcess is continuously spawning new threads when it is running. Verify that no new threads
  // get spawned - i.e. the process is not running anymore.
  const auto tids = orbit_base::GetTidsOfProcess(pid);
  absl::SleepFor(absl::Milliseconds(50));
  const auto tids_new = orbit_base::GetTidsOfProcess(pid);
  EXPECT_EQ(std::set<pid_t>(tids.begin(), tids.end()),
            std::set<pid_t>(tids_new.begin(), tids_new.end()));

  const auto detach_result = DetachAndContinueProcess(pid);
  ASSERT_FALSE(detach_result.has_error()) << detach_result.error().message();
}

}  // namespace orbit_user_space_instrumentation
