// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <gmock/gmock-matchers.h>
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
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TestUtils.h"
#include "RegisterState.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::HasErrorContaining;

AddressRange AddressRangeFromString(const std::string& string_address) {
  AddressRange result;
  const std::vector<std::string> addresses = absl::StrSplit(string_address, '-');
  if (addresses.size() != 2) {
    FATAL("Not an address range: %s", string_address);
  }
  if (!absl::numbers_internal::safe_strtou64_base(addresses[0], &result.start, 16)) {
    FATAL("Not a number: %s", addresses[0]);
  }
  if (!absl::numbers_internal::safe_strtou64_base(addresses[1], &result.end, 16)) {
    FATAL("Not a number: %s", addresses[1]);
  }
  return result;
}

// Get the address range of the first consecutive mappings. We can read this range but not more.
[[nodiscard]] ErrorMessageOr<AddressRange> GetFirstContinuousAddressRange(pid_t pid) {
  OUTCOME_TRY(maps, orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));
  const std::vector<std::string> lines = absl::StrSplit(maps, '\n', absl::SkipEmpty());
  bool is_first = true;
  AddressRange result;
  for (const auto& line : lines) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4) continue;
    const AddressRange current_range = AddressRangeFromString(tokens[0]);
    if (is_first) {
      result = current_range;
      is_first = false;
    } else {
      if (current_range.start == result.end) {
        result.end = current_range.end;
      }
    }
  }
  return result;
}

}  // namespace

TEST(AccessTraceesMemoryTest, ReadFailures) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  auto continuous_range_or_error = GetFirstContinuousAddressRange(pid);
  CHECK(continuous_range_or_error.has_value());
  const auto continuous_range = continuous_range_or_error.value();
  const uint64_t address = continuous_range.start;
  const uint64_t length = continuous_range.end - continuous_range.start;

  // Good read.
  ErrorMessageOr<std::vector<uint8_t>> result = ReadTraceesMemory(pid, address, length);
  EXPECT_TRUE(result.has_value());

  // Process does not exist.
  result = ReadTraceesMemory(-1, address, length);
  EXPECT_THAT(result, HasErrorContaining("Unable to open file"));

  // Read 0 bytes.
  EXPECT_DEATH(auto unused_result = ReadTraceesMemory(pid, address, 0), "Check failed");

  // Read past the end of the mappings.
  result = ReadTraceesMemory(pid, address, length + 1);
  EXPECT_THAT(result, HasErrorContaining("Only got"));

  // Read from bad address.
  result = ReadTraceesMemory(pid, 0, length);
  EXPECT_THAT(result, HasErrorContaining("Input/output error"));

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(AccessTraceesMemoryTest, WriteFailures) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  auto continuous_range_or_error = GetFirstContinuousAddressRange(pid);
  CHECK(continuous_range_or_error.has_value());
  const auto continuous_range = continuous_range_or_error.value();
  const uint64_t address = continuous_range.start;
  const uint64_t length = continuous_range.end - continuous_range.start;

  // Backup.
  auto backup = ReadTraceesMemory(pid, address, length);
  CHECK(backup.has_value());

  // Good write.
  std::vector<uint8_t> bytes(length, 0);
  ErrorMessageOr<void> result = WriteTraceesMemory(pid, address, bytes);
  EXPECT_FALSE(result.has_error());

  // Process does not exist.
  result = WriteTraceesMemory(-1, address, bytes);
  EXPECT_THAT(result, HasErrorContaining("Unable to open file"));

  // Write 0 bytes.
  EXPECT_DEATH(auto unused_result = WriteTraceesMemory(pid, address, std::vector<uint8_t>()),
               "Check failed");

  // Write past the end of the mappings.
  bytes.push_back(0);
  result = WriteTraceesMemory(pid, address, bytes);
  EXPECT_THAT(result, HasErrorContaining("Input/output error"));

  // Write to bad address.
  result = WriteTraceesMemory(pid, 0, bytes);
  EXPECT_THAT(result, HasErrorContaining("Input/output error"));

  // Restore, detach and end child.
  CHECK(!WriteTraceesMemory(pid, address, backup.value()).has_error());
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(AccessTraceesMemoryTest, ReadWriteRestore) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    // Child just runs an endless loop.
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  auto memory_region_or_error = GetFirstExecutableMemoryRegion(pid);
  CHECK(memory_region_or_error.has_value());
  const uint64_t address = memory_region_or_error.value().start;

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
  CHECK(WriteTraceesMemory(pid, address, backup.value()).has_value());
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation