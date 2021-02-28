// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <filesystem>
#include <string>
#include <vector>

#include "AllocateInTracee.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/WriteStringToFile.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::ReadFileToString;
using orbit_base::WriteStringToFile;

// Let the parent trace us, write address of `go_on` into file, then enter a breakpoint. While the
// child is stopped the parent can modify `go_on` in which case the child exits the endless loop
// when continued.
void Child(std::filesystem::path p) {
  CHECK(ptrace(PTRACE_TRACEME, 0, NULL, 0) != -1);
  volatile uint64_t go_on = 42;
  std::string s = absl::StrFormat("%p", &go_on);
  CHECK(WriteStringToFile(p, s));

  __asm__ __volatile__("int3\n\t");

  uint64_t data = 0;
  while (go_on == 42) {
    data++;
  }
  exit(0);
}

// Returns true if `tid` has a readable, writeable, and executable memory segment at `address`.
bool DoesAddressRangeExit(pid_t tid, uint64_t address) {
  auto result_read_maps = ReadFileToString(absl::StrFormat("/proc/%d/maps", tid));
  CHECK(result_read_maps);
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
  std::filesystem::path p = std::tmpnam(nullptr);
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    Child(p);
  }

  // Wait for child to break and get the address of the `go_on` variable on the child side.
  waitpid(pid, NULL, 0);
  auto result_go_on = ReadFileToString(p);
  CHECK(result_go_on);
  const uint64_t address_go_on = std::stoull(result_go_on.value(), nullptr, 16);

  // Continue child by detaching.
  CHECK(ptrace(PTRACE_DETACH, pid, nullptr, nullptr) != -1);

  // Stop the process again using our tooling.
  CHECK(AttachAndStopProcess(pid));

  // Allocate a megabyte in the tracee.
  uint64_t kMemorySize = 1024 * 1024;
  auto result_allocate = AllocateInTracee(pid, kMemorySize);
  CHECK(result_allocate);

  EXPECT_TRUE(DoesAddressRangeExit(pid, result_allocate.value()));

  CHECK(FreeInTracee(pid, result_allocate.value(), kMemorySize));

  EXPECT_FALSE(DoesAddressRangeExit(pid, result_allocate.value()));

  // Alter `go_on` so that the child will exit, continue, and wait for the exit to happen.
  CHECK(ptrace(PTRACE_POKEDATA, pid, address_go_on, 54) != -1);
  CHECK(DetachAndContinueProcess(pid));
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation