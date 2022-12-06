// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <random>
#include <vector>

#include "AccessTraceesMemory.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "TestUtils.h"
#include "UserSpaceInstrumentation/AddressRange.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

extern "C" int SomethingToDisassemble() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(1, 6);
  return dis(gen);
}

TEST(TestUtilTest, Disassemble) {
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    [[maybe_unused]] volatile int sum = 0;
    while (true) {
      sum += SomethingToDisassemble();
    }
  }

  // Stop the child process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  constexpr const char* kFunctionName = "SomethingToDisassemble";
  const AddressRange range = GetFunctionAbsoluteAddressRangeOrDie(kFunctionName);
  auto function_code_or_error = ReadTraceesMemory(pid, range.start, range.end - range.start);
  ORBIT_CHECK(!function_code_or_error.has_error());

  DumpDisassembly(function_code_or_error.value(), range.start);

  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(TestUtilTest, GetFunctionAddressRangeInFile) {
  constexpr const char* kFunctionName = "SomethingToDisassemble";
  AddressRange range = FindFunctionOrDie(kFunctionName).relative_address_range;
  EXPECT_NE(0, range.start);
  EXPECT_LT(range.start, range.end);
}

}  // namespace orbit_user_space_instrumentation