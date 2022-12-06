// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <csignal>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AllocateInTracee.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
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
    [[maybe_unused]] volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Stop the process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  // Allocation fails for invalid process.
  constexpr uint64_t kMemorySize = 1024u * 1024u;
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
    [[maybe_unused]] volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Stop the process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  constexpr uint64_t kMemorySize = 1024u * 1024u;
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

TEST(AllocateInTraceeTest, SyscallInTraceeFailsBecauseOfStrictSeccompMode) {
  std::array<int, 2> child_to_parent_pipe{};
  ORBIT_CHECK(pipe(child_to_parent_pipe.data()) == 0);

  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Close the read end of the pipe.
    ORBIT_CHECK(close(child_to_parent_pipe[0]) == 0);

    // Transition to strict seccomp mode.
    ORBIT_CHECK(syscall(SYS_seccomp, SECCOMP_SET_MODE_STRICT, 0, nullptr) == 0);

    // Send one byte to the parent to notify that the child has called seccomp. Note that the strict
    // seccomp mode still allows write.
    ORBIT_CHECK(write(child_to_parent_pipe[1], "a", 1) == 1);

    // Child just runs an endless loop.
    [[maybe_unused]] volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Close the write end of the pipe.
  ORBIT_CHECK(close(child_to_parent_pipe[1]) == 0);

  // Wait for the child to execute the seccomp syscall.
  std::array<char, 1> buf{};
  ORBIT_CHECK(read(child_to_parent_pipe[0], buf.data(), 1) == 1);

  // Stop the process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  constexpr uint64_t kMemorySize = 1024u * 1024u;
  // Allocation will fail because of seccomp.
  auto my_memory_or_error = MemoryInTracee::Create(pid, 0, kMemorySize);
  EXPECT_THAT(
      my_memory_or_error,
      HasError(absl::StrFormat(
          "This might be due to thread %d being in seccomp mode 1 (SECCOMP_MODE_STRICT).", pid)));

  // The forked process was killed because of seccomp and it cannot be waited for.
  ORBIT_CHECK(kill(pid, 0) != 0);
}

TEST(AllocateInTraceeTest, SyscallInTraceeFailsBecauseOfSeccompFilter) {
  std::array<int, 2> child_to_parent_pipe{};
  ORBIT_CHECK(pipe(child_to_parent_pipe.data()) == 0);

  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Close the read end of the pipe.
    ORBIT_CHECK(close(child_to_parent_pipe[0]) == 0);

    // "In order to use the SECCOMP_SET_MODE_FILTER operation, [...] the thread must already have
    // the no_new_privs bit set."
    ORBIT_CHECK(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == 0);

    // Set the following filter, which makes any system call other than write result in EPERM.
    //  line  OP   JT   JF   K
    // =================================
    //  0000: 0x20 0x00 0x00 0x00000000   ld  $data[0]
    //  0001: 0x15 0x00 0x01 0x00000001   jeq 1    true:0002 false:0003
    //  0002: 0x06 0x00 0x00 0x7fff0000   ret ALLOW
    //  0003: 0x06 0x00 0x00 0x00050001   ret ERRNO(1)
    std::array<sock_filter, 4> filter{
        sock_filter{.code = 0x20, .jt = 0x00, .jf = 0x00, .k = 0x00000000},
        sock_filter{.code = 0x15, .jt = 0x00, .jf = 0x01, .k = 0x00000001},
        sock_filter{.code = 0x06, .jt = 0x00, .jf = 0x00, .k = 0x7fff0000},
        sock_filter{.code = 0x06, .jt = 0x00, .jf = 0x00, .k = 0x00050001},
    };
    sock_fprog program{.len = filter.size(), .filter = filter.data()};
    ORBIT_CHECK(syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, 0, &program) == 0);

    // Send one byte to the parent to notify that the child has called seccomp.
    ORBIT_CHECK(write(child_to_parent_pipe[1], "a", 1) == 1);

    // Child just runs an endless loop.
    [[maybe_unused]] volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // Close the write end of the pipe.
  ORBIT_CHECK(close(child_to_parent_pipe[1]) == 0);

  // Wait for the child to execute the seccomp syscall.
  std::array<char, 1> buf{};
  ORBIT_CHECK(read(child_to_parent_pipe[0], buf.data(), 1) == 1);

  // Stop the process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  constexpr uint64_t kMemorySize = 1024u * 1024u;
  // Allocation will fail because of seccomp.
  auto my_memory_or_error = MemoryInTracee::Create(pid, 0, kMemorySize);
  EXPECT_THAT(
      my_memory_or_error,
      HasError(absl::StrFormat(
          "This might be due to thread %d being in seccomp mode 2 (SECCOMP_MODE_FILTER).", pid)));

  // Detach and end child.
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation