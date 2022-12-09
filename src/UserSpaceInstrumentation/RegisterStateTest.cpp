// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "RegisterState.h"
#include "TestUtils/TestUtils.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_test_utils::HasError;

// Let the parent trace us, write into rax and ymm0, then enter a breakpoint. While the child is
// stopped the parent modifies the registers and continues the child. The child then reads back the
// registers and verifies the modifications done by the parent. The exit code indicates the outcome
// of that verification.
void Child() {
  ORBIT_CHECK(ptrace(PTRACE_TRACEME, 0, nullptr, 0) != -1);

  uint64_t rax = 0xaabbccdd;
  std::array<uint8_t, 32> avx_bytes{};
  for (size_t i = 0; i < avx_bytes.size(); ++i) {
    avx_bytes[i] = i;
  }
  // The first three touch the fpu and move the memory to the registers. The "%0" and "%1" refer to
  // the addresses of "rax" and "avx_bytes" given in the second line from the bottom. "int3" just is
  // the breakpoint. The parent does "waitpid" for that. Line four and five move the registers back
  // into memory so they are available for verification below.
  __asm__ __volatile__(
      "flds -0x10(%%rsp)\n\t"
      "mov (%0), %%rax\n\t"
      "vmovups (%1), %%ymm0\n\t"
      "int3\n\t"
      "mov %%rax, (%0)\n\t"
      "vmovups %%ymm0, (%1)\n\t"
      :
      : "r"(&rax), "b"(avx_bytes.data())
      : "%rax", "%ymm0", "memory");

  if (rax != 0xaabbccdd + 0x11223344) {
    exit(1);
  }
  for (size_t i = 0; i < avx_bytes.size(); ++i) {
    if (avx_bytes[i] != 0x10 + i) {
      exit(1);
    }
  }
  exit(0);
}

}  // namespace

TEST(RegisterStateTest, BackupModifyRestore) {
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    Child();
  }

  // Wait for child to break.
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  ORBIT_CHECK(waited == pid);
  ORBIT_CHECK(WIFSTOPPED(status));
  ORBIT_CHECK(WSTOPSIG(status) == SIGTRAP);

  // Read child's registers and check values.
  RegisterState state;

  EXPECT_TRUE(state.BackupRegisters(pid).has_value());
  EXPECT_EQ(state.GetGeneralPurposeRegisters()->x86_64.rax, 0xaabbccdd);
  EXPECT_TRUE(state.Hasx87DataStored());
  EXPECT_TRUE(state.HasSseDataStored());
  EXPECT_TRUE(state.HasAvxDataStored());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(state.GetFxSave()->xmm[0].bytes[i], i);
    EXPECT_EQ(state.GetAvxHiRegisters()->ymm[0].bytes[i], i + 16);
  }

  // Modify rax and ymm0 and write them back to the child.
  state.GetGeneralPurposeRegisters()->x86_64.rax += 0x11223344;
  for (int i = 0; i < 16; ++i) {
    state.GetFxSave()->xmm[0].bytes[i] += 0x10;
    state.GetAvxHiRegisters()->ymm[0].bytes[i] += 0x10;
  }
  EXPECT_TRUE(state.RestoreRegisters().has_value());

  // Continue child.
  ORBIT_CHECK(ptrace(PT_CONTINUE, pid, 1, 0) == 0);

  // Wait for the child to exit. Exit status is zero if the modified registers could be verified.
  waited = waitpid(pid, &status, 0);
  ORBIT_CHECK(waited == pid);
  ORBIT_CHECK(WIFEXITED(status));
  EXPECT_EQ(WEXITSTATUS(status), 0);

  // After the process exited we get errors when backing up / restoring registers.
  EXPECT_THAT(state.RestoreRegisters(), HasError("PTRACE_SETREGSET failed to write NT_PRSTATUS"));
  EXPECT_THAT(state.BackupRegisters(pid), HasError("PTRACE_GETREGS, NT_PRSTATUS failed"));
}

}  // namespace orbit_user_space_instrumentation
