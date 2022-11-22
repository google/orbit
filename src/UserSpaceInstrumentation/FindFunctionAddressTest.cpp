// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <signal.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "FindFunctionAddress.h"
#include "GrpcProtos/module.pb.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

TEST(FindFunctionAddressTest, FindFunctionAddress) {
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Stop the child process using our tooling.
  ORBIT_CHECK(AttachAndStopProcess(pid).has_value());

  auto modules_or_error = orbit_module_utils::ReadModules(pid);
  ASSERT_THAT(modules_or_error, orbit_test_utils::HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

  auto function_address_or_error = FindFunctionAddress(modules, "libc.so.6", "printf");
  ASSERT_TRUE(function_address_or_error.has_value()) << function_address_or_error.error().message();

  function_address_or_error = FindFunctionAddress(modules, "libc.so.6", "NOT_A_SYMBOL");
  ASSERT_TRUE(function_address_or_error.has_error());
  EXPECT_TRUE(absl::StrContains(function_address_or_error.error().message(),
                                "Unable to locate function symbol"));

  function_address_or_error = FindFunctionAddress(modules, "NOT_A_LIB-", "printf");
  ASSERT_TRUE(function_address_or_error.has_error());
  EXPECT_TRUE(absl::StrContains(function_address_or_error.error().message(),
                                "There is no module \"NOT_A_LIB-\" in the target process"));

  // Detach and end child.
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation