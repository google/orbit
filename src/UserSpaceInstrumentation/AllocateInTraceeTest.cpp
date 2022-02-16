// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>

#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "AllocateInTracee.h"
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
  ORBIT_CHECK(maps_or_error.has_value());
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
  ORBIT_CHECK(pid != -1);
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
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  // Allocation fails for invalid process.
  constexpr uint64_t kMemorySize = 1024 * 1024;
  auto my_memory_or_error = MemoryInTracee::Create(-1, 0, kMemorySize);
  EXPECT_THAT(my_memory_or_error, HasError("No such process"));

  // Allocation fails for non page aligned address.
  my_memory_or_error = MemoryInTracee::Create(pid, 1, kMemorySize);
  EXPECT_THAT(my_memory_or_error, HasError("but got memory at a different address"));

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
  ORBIT_CHECK(mmap_min_addr_or_error.has_value());
  uint64_t mmap_min_addr = 0;
  ORBIT_CHECK(absl::SimpleAtoi(mmap_min_addr_or_error.value(), &mmap_min_addr));
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
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(AllocateInTraceeTest, AutomaticAllocateAndFree) {
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
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
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

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
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(AllocateInTraceeTest, ReadSeccompModeOfThread) {
  std::optional<int> seccomp_mode = ReadSeccompModeOfThread(getpid());
  ASSERT_TRUE(seccomp_mode.has_value());
  EXPECT_THAT(seccomp_mode.value(),
              testing::AnyOf(SECCOMP_MODE_DISABLED, SECCOMP_MODE_STRICT, SECCOMP_MODE_FILTER));
}

TEST(AllocateInTraceeTest, SyscallInTraceeFailsBecauseOfSeccomp) {
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Transition to strict seccomp mode.
    int64_t seccomp_result = syscall(SYS_seccomp, SECCOMP_SET_MODE_STRICT, 0, nullptr);
    ORBIT_CHECK(seccomp_result == 0);

    // Child just runs an endless loop.
    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Give the child enough time to execute the seccomp syscall.
  std::this_thread::sleep_for(std::chrono::milliseconds{1000});

  int wstatus = 0;
  pid_t waited_pid = waitpid(pid, &wstatus, WNOHANG);
  ORBIT_CHECK(waited_pid != -1);
  if (waited_pid != 0) {
    // ORBIT_CHECK(seccomp_result == 0) failed in the child.
    ORBIT_CHECK(waited_pid == pid);
    ORBIT_CHECK(WIFSIGNALED(wstatus));
    ORBIT_CHECK(WTERMSIG(wstatus) == SIGABRT);
    ORBIT_ERROR("Child failed to call seccomp");
    GTEST_SKIP();
  }
  ORBIT_CHECK(kill(pid, 0) == 0);

  // Stop the process using our tooling.
  auto attach_and_stop_result = AttachAndStopProcess(pid);
  if (attach_and_stop_result.has_error()) {
    ORBIT_FATAL("attach_and_stop_result.has_error(): %s", attach_and_stop_result.error().message());
  }

  constexpr uint64_t kMemorySize = 1024 * 1024;

  // Allocation will fail because of seccomp.
  auto my_memory_or_error = MemoryInTracee::Create(pid, 0, kMemorySize);
  EXPECT_THAT(
      my_memory_or_error,
      HasError(absl::StrFormat(
          "This might be due to thread %d being in seccomp mode 1 (SECCOMP_MODE_STRICT).", pid)));

  // The forked process was killed because of seccomp and it cannot be waited for.
  ORBIT_CHECK(kill(pid, 0) != 0);
}

}  // namespace orbit_user_space_instrumentation