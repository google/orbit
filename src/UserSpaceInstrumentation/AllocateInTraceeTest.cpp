// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>
#include <filesystem>
#include <string>
#include <vector>

#include "AllocateInTracee.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::ReadFileToString;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

enum class ProtState { kWrite, kExec, kAny };

// Returns true if the target process has a writeable (or executable; depending on `state`) memory
// segment at `address`.
[[nodiscard]] bool ProcessHasMapAtAddress(pid_t pid, uint64_t address, ProtState state) {
  auto maps_or_error = ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  CHECK(maps_or_error.has_value());
  std::vector<std::string> lines = absl::StrSplit(maps_or_error.value(), '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (state == ProtState::kWrite) {
      if (tokens.size() < 2 || tokens[1].size() < 2 || tokens[1][1] != 'w') {
        continue;
      }
    } else if (state == ProtState::kExec) {
      if (tokens.size() < 2 || tokens[1].size() < 3 || tokens[1][2] != 'x') {
        continue;
      }
    }
    std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) {
      continue;
    }
    if (std::stoull(addresses[0], nullptr, 16) == address) {
      return true;
    }
  }
  return false;
}

// Returns true if the target process has a writeable (or executable; depending on `state`) memory
// segment at the address of `memory`.
[[nodiscard]] bool ProcessHasMapAtAddress(const MemoryInTracee& memory, ProtState state) {
  return ProcessHasMapAtAddress(memory.GetPid(), memory.GetAddress(), state);
}

}  // namespace

TEST(AllocateInTraceeTest, AllocateAndFree) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Child just runs an endless loop.
    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Stop the process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  // Allocation fails for invalid process.
  constexpr uint64_t kMemorySize = 1024 * 1024;
  auto my_memory_or_error = MemoryInTracee::Create(-1, 0, kMemorySize);
  EXPECT_THAT(my_memory_or_error, HasError("No such process"));

  // Allocation fails for non page aligned address.
  my_memory_or_error = MemoryInTracee::Create(pid, 1, kMemorySize);
  EXPECT_THAT(my_memory_or_error, HasError("but got memory at a different adress"));

  // Allocation fails for ridiculous size.
  my_memory_or_error = MemoryInTracee::Create(pid, 1, 1ull << 63);
  EXPECT_THAT(my_memory_or_error, HasError("Syscall failed. Return value: Cannot allocate memory"));

  // Allocate a megabyte in the tracee.
  my_memory_or_error = MemoryInTracee::Create(pid, 0, kMemorySize);
  ASSERT_THAT(my_memory_or_error, HasNoError());
  EXPECT_TRUE(ProcessHasMapAtAddress(*my_memory_or_error.value(), ProtState::kWrite));

  // Free the memory.
  ASSERT_THAT(my_memory_or_error.value()->Free(), HasNoError());

  // Allocate a megabyte at a low memory position.
  auto mmap_min_addr_or_error = ReadFileToString("/proc/sys/vm/mmap_min_addr");
  CHECK(mmap_min_addr_or_error.has_value());
  uint64_t mmap_min_addr = 0;
  CHECK(absl::SimpleAtoi(mmap_min_addr_or_error.value(), &mmap_min_addr));
  my_memory_or_error = MemoryInTracee::Create(pid, mmap_min_addr, kMemorySize);
  ASSERT_THAT(my_memory_or_error, HasNoError());
  auto my_memory = std::move(my_memory_or_error.value());
  EXPECT_TRUE(ProcessHasMapAtAddress(*my_memory, ProtState::kWrite));

  // Make memory executable.
  ASSERT_THAT(my_memory->EnsureMemoryExecutable(), HasNoError());
  EXPECT_TRUE(ProcessHasMapAtAddress(*my_memory, ProtState::kExec));

  // Make memory writable again.
  ASSERT_THAT(my_memory->EnsureMemoryWritable(), HasNoError());
  EXPECT_TRUE(ProcessHasMapAtAddress(*my_memory, ProtState::kWrite));

  // Free the memory.
  uint64_t address = my_memory->GetAddress();
  ASSERT_THAT(my_memory->Free(), HasNoError());
  EXPECT_FALSE(ProcessHasMapAtAddress(pid, address, ProtState::kAny));

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(AllocateInTraceeTest, AutomaticAllocateAndFree) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Child just runs an endless loop.
    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Stop the process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  constexpr uint64_t kMemorySize = 1024 * 1024;
  uint64_t address = 0;
  {
    auto automatic_memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, kMemorySize);
    ASSERT_THAT(automatic_memory_or_error, HasNoError());
    auto automatic_memory = std::move(automatic_memory_or_error.value());
    EXPECT_TRUE(ProcessHasMapAtAddress(*automatic_memory, ProtState::kWrite));
    address = automatic_memory->GetAddress();
  }
  EXPECT_FALSE(ProcessHasMapAtAddress(pid, address, ProtState::kAny));

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation