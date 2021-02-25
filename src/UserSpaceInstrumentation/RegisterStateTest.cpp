// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "OrbitBase/Logging.h"
#include "UserSpaceInstrumentation/RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

// Let the parent trace us, write into rax and ymm0, then enter a breakpoint. While the child is
// stopped the parent modifies the register and continues the child. The child then reads back the
// registers and verifies the modifications done by the parent. The exit code indicates the outcome
// of that verification.
void Child() {
  CHECK(ptrace(PTRACE_TRACEME, 0, NULL, 0) != -1);

  uint64_t rax = 0xaabbccdd;
  uint8_t avx_bytes[32];
  for (int i = 0; i < 32; ++i) {
    avx_bytes[i] = i;
  }
  // The first two lines move the memory to the registers. The "%0" and "%1" refer to the adresses
  // of "rax" and "avx_bytes" given in the second line from the bottom. "int3" just is the
  // breakpoint. The parent does "waitpid" for that. line four and five move the registers back into
  // memory so they are available for verification below.
  __asm__ __volatile__(
      "mov (%0), %%rax\n\t"
      "vmovups (%1), %%ymm0\n\t"
      "int3\n\t"
      "mov %%rax, (%0)\n\t"
      "vmovups %%ymm0, (%1)\n\t"
      :
      : "r"(&rax), "b"(avx_bytes)
      : "%rax", "%ymm0", "memory");

  if (rax != 0xaabbccdd + 0x11223344) {
    exit(1);
  }
  for (int i = 0; i < 32; ++i) {
    if (avx_bytes[i] != 0x10 + i) {
      exit(1);
    }
  }
  exit(0);
}

}  // namespace

TEST(RegisterStateTest, BackupModifyRestore) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    Child();
  }

  // Wait for child to break.
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  CHECK(waited == pid);
  CHECK(WIFSTOPPED(status));
  CHECK(WSTOPSIG(status) == SIGTRAP);

  // Read child's registers and check values.
  RegisterState s;
  EXPECT_TRUE(s.BackupRegisters(pid));
  EXPECT_EQ(s.GetGeneralPurposeRegisters()->x86_64.rax, 0xaabbccdd);
  EXPECT_TRUE(s.HasSseDataStored());
  EXPECT_TRUE(s.HasAvxDataStored());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(s.GetFxSave()->xmm[0].bytes[i], i);
    EXPECT_EQ(s.GetAvxHiRegisters()->ymm[0].bytes[i], i + 16);
  }

  // Modify rax and ymm0 and write them back to the child.
  s.GetGeneralPurposeRegisters()->x86_64.rax += 0x11223344;
  for (int i = 0; i < 16; ++i) {
    s.GetFxSave()->xmm[0].bytes[i] += 0x10;
    s.GetAvxHiRegisters()->ymm[0].bytes[i] += 0x10;
  }
  EXPECT_TRUE(s.RestoreRegisters());

  // Continue child.
  CHECK(ptrace(PT_CONTINUE, pid, 1, 0) == 0);

  // Wait for the child to exit. Exit status is zero if the modified registers could be verified.
  waited = waitpid(pid, &status, 0);
  CHECK(waited == pid);
  CHECK(WIFEXITED(status));
  EXPECT_EQ(WEXITSTATUS(status), 0);
}

}  // namespace orbit_user_space_instrumentation
