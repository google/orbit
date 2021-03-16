// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <random>
#include <vector>

#include "AccessTraceesMemory.h"
#include "Attach.h"
#include "OrbitBase/Logging.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

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

  auto result_memory_region = GetFirstExecutableMemoryRegion(pid);
  CHECK(result_memory_region.has_value());
  const uint64_t address_start = result_memory_region.value().first;

  constexpr uint64_t kMemorySize = 4 * 1024;
  auto result_backup = ReadTraceesMemory(pid, address_start, kMemorySize);
  CHECK(result_backup.has_value());

  std::vector<uint8_t> new_data(kMemorySize);
  std::mt19937 engine{std::random_device()()};
  std::uniform_int_distribution<uint8_t> distribution{0x00, 0xff};
  std::generate(std::begin(new_data), std::end(new_data),
                [&distribution, &engine]() { return distribution(engine); });

  CHECK(!WriteTraceesMemory(pid, address_start, new_data).has_error());

  auto result_read_back = ReadTraceesMemory(pid, address_start, kMemorySize);
  CHECK(result_read_back.has_value());

  EXPECT_EQ(new_data, result_read_back.value());

  CHECK(WriteTraceesMemory(pid, address_start, result_backup.value()).has_value());

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation