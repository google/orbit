// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>
#include <filesystem>
#include <string>
#include <vector>

#include "AllocateInTracee.h"
#include "Attach.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::ReadFileToString;

// Returns true if `pid` has a readable, writeable, and executable memory segment at `address`.
[[nodiscard]] bool ProcessHasRwxMapAtAddress(pid_t pid, uint64_t address) {
  auto result_read_maps = ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  CHECK(result_read_maps.has_value());
  std::vector<std::string> lines =
      absl::StrSplit(result_read_maps.value(), '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4 || tokens[1][0] != 'r' || tokens[1][1] != 'w' ||
        tokens[1][2] != 'x') {
      continue;
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

}  // namespace

TEST(AllocateInTraceeTest, AllocateAndFree) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

  // Allocate a megabyte in the tracee.
  constexpr uint64_t kMemorySize = 1024 * 1024;
  auto result_allocate = AllocateInTracee(pid, 0, kMemorySize);
  ASSERT_TRUE(result_allocate.has_value());
  EXPECT_TRUE(ProcessHasRwxMapAtAddress(pid, result_allocate.value()));

  // Free the memory.
  ASSERT_FALSE(FreeInTracee(pid, result_allocate.value(), kMemorySize).has_error());
  EXPECT_FALSE(ProcessHasRwxMapAtAddress(pid, result_allocate.value()));

  // Allocate a megabyte at a low memory position.
  auto mmap_min_addr_or_error = ReadFileToString("/proc/sys/vm/mmap_min_addr");
  CHECK(mmap_min_addr_or_error.has_value());
  uint64_t mmap_min_addr = 0;
  CHECK(absl::SimpleAtoi(mmap_min_addr_or_error.value(), &mmap_min_addr));
  result_allocate = AllocateInTracee(pid, mmap_min_addr, kMemorySize);
  ASSERT_TRUE(result_allocate.has_value());
  EXPECT_TRUE(ProcessHasRwxMapAtAddress(pid, result_allocate.value()));

  // Free the memory.
  ASSERT_FALSE(FreeInTracee(pid, result_allocate.value(), kMemorySize).has_error());
  EXPECT_FALSE(ProcessHasRwxMapAtAddress(pid, result_allocate.value()));

  // Detach and end child.
  CHECK(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation