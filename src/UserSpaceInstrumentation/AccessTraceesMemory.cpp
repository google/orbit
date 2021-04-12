// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessTraceesMemory.h"

#include <absl/base/casts.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <sys/ptrace.h>

#include <cerrno>
#include <cstring>
#include <string>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_user_space_instrumentation {

using orbit_base::ReadFileToString;

[[nodiscard]] ErrorMessageOr<std::vector<uint8_t>> ReadTraceesMemory(pid_t pid,
                                                                     uint64_t address_start,
                                                                     uint64_t length) {
  if (length == 0) {
    return ErrorMessage("Tried to read 0 bytes.");
  }

  OUTCOME_TRY(fd, orbit_base::OpenFileForReading(absl::StrFormat("/proc/%d/mem", pid)));

  std::vector<uint8_t> data(length);
  ssize_t result = pread(fd.get(), data.data(), length, address_start);

  if (result == -1) {
    return ErrorMessage(
        absl::StrFormat("Failed to read from memory file of process %d with errno %d: \"%s\"", pid,
                        errno, SafeStrerror(errno)));
  }

  if (result < static_cast<ssize_t>(length)) {
    return ErrorMessage(absl::StrFormat(
        "Failed to read %u bytes from memory file of process %d. Only got %d bytes.", length, pid,
        result));
  }

  return data;
}

[[nodiscard]] ErrorMessageOr<void> WriteTraceesMemory(pid_t pid, uint64_t address_start,
                                                      const std::vector<uint8_t>& bytes) {
  if (bytes.empty()) {
    return ErrorMessage("Tried to write 0 bytes.");
  }
  OUTCOME_TRY(fd, orbit_base::OpenFileForWriting(absl::StrFormat("/proc/%d/mem", pid)));
  ssize_t result = pwrite(fd.get(), bytes.data(), bytes.size(), address_start);

  if (result == -1) {
    return ErrorMessage(
        absl::StrFormat("Failed to write to memory file of process pid %d with errno %d: \"%s\"",
                        pid, errno, SafeStrerror(errno)));
  }

  if (result < static_cast<ssize_t>(bytes.size())) {
    return ErrorMessage(absl::StrFormat(
        "Failed to write %u bytes to memory file of process pid %d. Only wrote %d bytes.",
        bytes.size(), pid, result));
  }

  return outcome::success();
}

[[nodiscard]] ErrorMessageOr<AddressRange> GetFirstExecutableMemoryRegion(
    pid_t pid, uint64_t exclude_address) {
  auto read_maps_or_error = ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  if (read_maps_or_error.has_error()) {
    return read_maps_or_error.error();
  }
  const std::vector<std::string> lines =
      absl::StrSplit(read_maps_or_error.value(), '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4 || tokens[1][2] != 'x') continue;
    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;
    AddressRange result;
    if (!absl::numbers_internal::safe_strtou64_base(addresses[0], &result.first, 16)) continue;
    if (!absl::numbers_internal::safe_strtou64_base(addresses[1], &result.second, 16)) continue;
    if (exclude_address >= result.first && exclude_address < result.second) continue;
    return result;
  }
  return ErrorMessage(absl::StrFormat("Unable to locate executable memory area in pid: %d", pid));
}

}  // namespace orbit_user_space_instrumentation