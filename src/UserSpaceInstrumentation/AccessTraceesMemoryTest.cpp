// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <vector>

#include "AccessTraceesMemory.h"
#include "OrbitBase/Logging.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {}  // namespace

TEST(AccessTraceesMemoryTest, ReadWriteRestore) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

  uint64_t address_start = 0;
  uint64_t address_end = 0;
  auto result_memory_region = GetFirstExecutableMemoryRegion(pid, &address_start, &address_end);
  CHECK(result_memory_region.has_value());

  std::vector<uint8_t> original;
  CHECK(ReadTraceesMemory(pid, address_start, address_end - address_start, &original).has_value());

  std::vector<uint8_t> new_data(original.size(), 42);
  CHECK(WriteTraceesMemory(pid, address_start, new_data).has_value());

  std::vector<uint8_t> read_back;
  CHECK(ReadTraceesMemory(pid, address_start, address_end - address_start, &read_back).has_value());

  EXPECT_EQ(new_data, read_back);

  CHECK(WriteTraceesMemory(pid, address_start, original).has_value());

  // Detach and end child.
  CHECK(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation