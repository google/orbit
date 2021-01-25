// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>

#include <set>
#include <vector>

#include "DummyProcessForTest.h"
#include "OrbitBase/GetProcessIds.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "absl/time/clock.h"
#include "gtest/gtest.h"

namespace orbit_user_space_instrumentation {

TEST(AttachTest, AttachAndStopProcess) {
  DummyProcessForTest dummy_process;
  const pid_t pid = dummy_process.pid();

  const auto result = AttachAndStopProcess(pid);
  ASSERT_TRUE(result) << result.error().message();

  // Verify that no new threads get spawned - i.e. the process is not running anymore.
  const auto tids = orbit_base::GetTidsOfProcess(pid);
  absl::SleepFor(absl::Milliseconds(50));
  const auto tids_new = orbit_base::GetTidsOfProcess(pid);
  EXPECT_EQ(std::set<pid_t>(tids.begin(), tids.end()),
            std::set<pid_t>(tids_new.begin(), tids_new.end()));

  DetachAndContinueProcess(pid);
}

}  // namespace orbit_user_space_instrumentation
