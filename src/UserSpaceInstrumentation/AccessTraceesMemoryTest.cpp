// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
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
#include "OrbitBase/ReadFileToString.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

// Get the adress range of the first consecutive mappings. We can read this range but not more.
[[nodiscard]] ErrorMessageOr<AddressRange> GetFirstContinuesAdressRange(pid_t pid) {
  OUTCOME_TRY(maps_or_error, orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));
  const std::vector<std::string> lines = absl::StrSplit(maps_or_error, '\n', absl::SkipEmpty());
  bool is_first = true;
  AddressRange result;
  for (const auto& line : lines) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4) continue;
    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;
    if (is_first) {
      if (!absl::numbers_internal::safe_strtou64_base(addresses[0], &result.first, 16)) continue;
      if (!absl::numbers_internal::safe_strtou64_base(addresses[1], &result.second, 16)) continue;
      is_first = false;
    } else {
      AddressRange current;
      if (!absl::numbers_internal::safe_strtou64_base(addresses[0], &current.first, 16)) continue;
      if (!absl::numbers_internal::safe_strtou64_base(addresses[1], &current.second, 16)) continue;
      if (current.first == result.second) {
        result.second = current.second;
      }
    }
  }
  return result;
}

}  // namespace

TEST(AccessTraceesMemoryTest, ReadFailures) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  auto continues_range_or_error = GetFirstContinuesAdressRange(pid);
  ASSERT_TRUE(continues_range_or_error.has_value());
  const auto continues_range = continues_range_or_error.value();
  const uint64_t address = continues_range.first;
  const uint64_t length = continues_range.second - continues_range.first;

  // Good read.
  auto result = ReadTraceesMemory(pid, address, length);
  EXPECT_TRUE(result.has_value());

  // Process does not exist.
  result = ReadTraceesMemory(-1, address, length);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Unable to open file"));

  // Read 0 bytes.
  result = ReadTraceesMemory(pid, address, 0);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Tried to read 0 bytes."));

  // Read past the end of the mappings.
  result = ReadTraceesMemory(pid, address, length + 1);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Only got"));

  // Read from bad address.
  result = ReadTraceesMemory(pid, 0, length);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(
      absl::StrContains(result.error().message(), "Failed to read from memory file of process"));

  // Detach and end child.
  ASSERT_TRUE(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(AccessTraceesMemoryTest, WriteFailures) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  auto continues_range_or_error = GetFirstContinuesAdressRange(pid);
  ASSERT_TRUE(continues_range_or_error.has_value());
  const auto continues_range = continues_range_or_error.value();
  const uint64_t address = continues_range.first;
  const uint64_t length = continues_range.second - continues_range.first;

  // Backup.
  auto backup = ReadTraceesMemory(pid, address, length);
  ASSERT_TRUE(backup.has_value());

  // Good write.
  std::vector<uint8_t> bytes(length, 0);
  auto result = WriteTraceesMemory(pid, address, bytes);
  EXPECT_FALSE(result.has_error());

  // Process does not exist.
  result = WriteTraceesMemory(-1, address, bytes);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Unable to open file"));

  // Write 0 bytes.
  result = WriteTraceesMemory(pid, address, std::vector<uint8_t>());
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Tried to write 0 bytes."));

  // Read past the end of the mappings.
  bytes.push_back(0);
  result = WriteTraceesMemory(pid, address, bytes);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Only wrote"));

  // Read from bad address.
  result = WriteTraceesMemory(pid, 0, bytes);
  ASSERT_TRUE(result.has_error());
  EXPECT_TRUE(absl::StrContains(result.error().message(), "Failed to write to memory file"));

  // Restore, detach and end child.
  EXPECT_FALSE(WriteTraceesMemory(pid, address, backup.value()).has_error());
  ASSERT_TRUE(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(AccessTraceesMemoryTest, ReadWriteRestore) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  auto memory_region_or_error = GetFirstExecutableMemoryRegion(pid);
  ASSERT_TRUE(memory_region_or_error.has_value());
  const uint64_t address = memory_region_or_error.value().first;

  constexpr uint64_t kMemorySize = 4 * 1024;
  auto backup = ReadTraceesMemory(pid, address, kMemorySize);
  ASSERT_TRUE(backup.has_value());

  std::vector<uint8_t> new_data(kMemorySize);
  std::mt19937 engine{std::random_device()()};
  std::uniform_int_distribution<uint8_t> distribution{0x00, 0xff};
  std::generate(std::begin(new_data), std::end(new_data),
                [&distribution, &engine]() { return distribution(engine); });

  ASSERT_FALSE(WriteTraceesMemory(pid, address, new_data).has_error());

  auto read_back_or_error = ReadTraceesMemory(pid, address, kMemorySize);
  ASSERT_TRUE(read_back_or_error.has_value());
  EXPECT_EQ(new_data, read_back_or_error.value());

  // Restore, detach and end child.
  ASSERT_TRUE(WriteTraceesMemory(pid, address, backup.value()).has_value());
  ASSERT_FALSE(DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation