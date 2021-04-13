// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gtest/gtest.h>
#include <sys/wait.h>

#include <csignal>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "MachineCode.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

TEST(ExecuteMachineCodeTest, ExecuteMachineCode) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  // Allocate a small chunk of memory.
  constexpr uint64_t kScratchPadSize = 1024;
  ErrorMessageOr<uint64_t> address_or_error = AllocateInTracee(pid, 0, kScratchPadSize);
  ASSERT_TRUE(address_or_error.has_value());
  const uint64_t address = address_or_error.value();

  // This code moves a constant into rax and enters a breakpoint. The value in rax is interpreted as
  // a return value.
  // movabs rax, 0x4242424242424242     48 b8 0x4242424242424242
  // int 3                              cc
  MachineCode code;
  code.AppendBytes({0x48, 0xb8}).AppendImmediate64(0x4242424242424242).AppendBytes({0xcc});
  ErrorMessageOr<uint64_t> result_or_error = ExecuteMachineCode(pid, address, code);
  ASSERT_FALSE(result_or_error.has_error());
  EXPECT_EQ(0x4242424242424242, result_or_error.value());

  ASSERT_FALSE(FreeInTracee(pid, address, kScratchPadSize).has_error());

  ASSERT_TRUE(DetachAndContinueProcess(pid).has_value());

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation