// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessTraceesMemory.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>

#include <algorithm>
#include <string>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_user_space_instrumentation {

using orbit_base::ReadFileToString;

[[nodiscard]] ErrorMessageOr<std::vector<uint8_t>> ReadTraceesMemory(pid_t pid,
                                                                     uint64_t start_address,
                                                                     uint64_t length) {
  ORBIT_CHECK(length != 0);

  OUTCOME_TRY(auto&& fd, orbit_base::OpenFileForReading(absl::StrFormat("/proc/%d/mem", pid)));

  std::vector<uint8_t> bytes(length);
  OUTCOME_TRY(auto&& result, ReadFullyAtOffset(fd, bytes.data(), length, start_address));

  if (result < length) {
    return ErrorMessage(absl::StrFormat(
        "Failed to read %u bytes from memory file of process %d. Only got %d bytes.", length, pid,
        result));
  }

  return bytes;
}

[[nodiscard]] ErrorMessageOr<void> WriteTraceesMemory(pid_t pid, uint64_t start_address,
                                                      absl::Span<const uint8_t> bytes) {
  ORBIT_CHECK(!bytes.empty());

  OUTCOME_TRY(auto&& fd, orbit_base::OpenFileForWriting(absl::StrFormat("/proc/%d/mem", pid)));

  OUTCOME_TRY(WriteFullyAtOffset(fd, bytes.data(), bytes.size(), start_address));

  return outcome::success();
}

[[nodiscard]] ErrorMessageOr<AddressRange> GetExistingExecutableMemoryRegion(
    pid_t pid, uint64_t exclude_address) {
  OUTCOME_TRY(auto&& maps, ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));
  const std::vector<std::string> lines = absl::StrSplit(maps, '\n', absl::SkipEmpty());
  // We pick the executable memory region with the highest address. This is to work around
  // http://b/214052981, which sees Wine use a seccomp filter to trap all syscalls coming from low
  // addresses, i.e., where it has loaded Windows DLLs and the game's .exe.
  for (auto line_it = lines.rbegin(); line_it != lines.rend(); ++line_it) {
    const std::string& line = *line_it;
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipWhitespace());
    if (tokens.size() < 2 || tokens[1].size() != 4 || tokens[1][2] != 'x') continue;
    // Writing to [vsyscall] or [uprobes] fails with EIO (Input/output error).
    if (tokens.size() >= 6 && (tokens[5] == "[vsyscall]" || tokens[5] == "[uprobes]")) continue;
    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;
    AddressRange result{};
    if (!absl::numbers_internal::safe_strtou64_base(addresses[0], &result.start, 16)) continue;
    if (!absl::numbers_internal::safe_strtou64_base(addresses[1], &result.end, 16)) continue;
    if (exclude_address >= result.start && exclude_address < result.end) continue;
    return result;
  }
  return ErrorMessage(absl::StrFormat("Unable to locate executable memory area in pid: %d", pid));
}

}  // namespace orbit_user_space_instrumentation