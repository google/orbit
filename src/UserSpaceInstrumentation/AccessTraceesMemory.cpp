// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessTraceesMemory.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <sys/ptrace.h>

#include <cerrno>
#include <cstring>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_user_space_instrumentation {

using orbit_base::ReadFileToString;

[[nodiscard]] ErrorMessageOr<void> ReadTraceesMemory(pid_t pid, uint64_t address_start,
                                                     uint64_t length, std::vector<uint8_t>* bytes) {
  // Round up length to next multiple of eight.
  length = ((length + 7) / 8) * 8;
  bytes->resize(length);
  for (size_t i = 0; i < length / 8; i++) {
    const uint64_t data = ptrace(PTRACE_PEEKDATA, pid, address_start + i * 8, NULL);
    if (errno) {
      return ErrorMessage(absl::StrFormat("Failed to PTRACE_PEEKDATA with errno %d: \"%s\"", errno,
                                          SafeStrerror(errno)));
    }
    std::memcpy(bytes->data() + (i * sizeof(uint64_t)), &data, 8);
  }
  return outcome::success();
}

[[nodiscard]] ErrorMessageOr<void> WriteTraceesMemory(pid_t pid, uint64_t address_start,
                                                      const std::vector<uint8_t>& bytes) {
  size_t pos = 0;
  do {
    // Pack 8 byte for writing into `data`.
    uint64_t data = 0;
    std::memcpy(&data, bytes.data() + pos, sizeof(uint64_t));
    if (ptrace(PTRACE_POKEDATA, pid, address_start + pos, data) == -1) {
      ErrorMessage(absl::StrFormat("Unable to write data into tracees (pid: %d) memory.", pid));
    }
    pos += sizeof(uint64_t);
  } while (pos < bytes.size());
  return outcome::success();
}

[[nodiscard]] ErrorMessageOr<void> GetFirstExecutableMemoryRegion(pid_t pid, uint64_t* addr_start,
                                                                  uint64_t* addr_end) {
  auto result_read_maps = ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  if (result_read_maps.has_error()) {
    return result_read_maps.error();
  }
  const std::vector<std::string> lines =
      absl::StrSplit(result_read_maps.value(), '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4 || tokens[1][2] != 'x') continue;
    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;
    if (!absl::numbers_internal::safe_strtou64_base(addresses[0], addr_start, 16)) continue;
    if (!absl::numbers_internal::safe_strtou64_base(addresses[1], addr_end, 16)) continue;
    return outcome::success();
  }
  return ErrorMessage(absl::StrFormat("Unable to locate executable memory area in pid: %d", pid));
}

}  // namespace orbit_user_space_instrumentation