// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <memory>
#include <utility>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "MachineCode.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

using orbit_test_utils::HasNoError;

TEST(ExecuteMachineCodeTest, ExecuteMachineCode) {
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
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  {
    // Allocate a small chunk of memory.
    constexpr uint64_t kScratchPadSize = 1024;
    auto memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, kScratchPadSize);
    ORBIT_CHECK(memory_or_error.has_value());
    auto memory = std::move(memory_or_error.value());

    // This code moves a constant into rax and enters a breakpoint. The value in rax is interpreted
    // as a return value.
    // movabs rax, 0x4242424242424242     48 b8 0x4242424242424242
    // int 3 cc
    MachineCode code;
    code.AppendBytes({0x48, 0xb8}).AppendImmediate64(0x4242424242424242).AppendBytes({0xcc});
    ErrorMessageOr<uint64_t> result_or_error = ExecuteMachineCode(*memory, code);
    ASSERT_THAT(result_or_error, HasNoError());
    EXPECT_EQ(0x4242424242424242, result_or_error.value());
  }

  // Cleanup, end child process.
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation