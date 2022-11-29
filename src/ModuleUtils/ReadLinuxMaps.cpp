// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ModuleUtils/ReadLinuxMaps.h"

#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_module_utils {

ErrorMessageOr<std::string> ReadMaps(pid_t pid) {
  const std::filesystem::path proc_pid_maps_path{absl::StrFormat("/proc/%i/maps", pid)};
  OUTCOME_TRY(auto&& proc_pid_maps_content, orbit_base::ReadFileToString(proc_pid_maps_path));
  return std::move(proc_pid_maps_content);
}

std::vector<LinuxMemoryMapping> ParseMaps(std::string_view proc_pid_maps_content) {
  const std::vector<std::string> proc_pid_maps_lines = absl::StrSplit(proc_pid_maps_content, '\n');
  std::vector<LinuxMemoryMapping> result;

  for (const std::string& line : proc_pid_maps_lines) {
    // The number of spaces from the inode to the path is variable, and the path can contain spaces,
    // so we need to limit the number of splits and remove leading spaces from the path separately.
    std::vector<std::string> tokens = absl::StrSplit(line, absl::MaxSplits(' ', 5));
    if (tokens.size() < 5) continue;
    ORBIT_CHECK(tokens.size() == 5 || tokens.size() == 6);

    const std::vector<std::string> start_and_end = absl::StrSplit(tokens[0], '-');
    if (start_and_end.size() != 2) continue;
    const uint64_t start = std::stoull(start_and_end[0], nullptr, 16);
    const uint64_t end = std::stoull(start_and_end[1], nullptr, 16);

    const uint64_t offset = std::stoull(tokens[2], nullptr, 16);

    if (tokens[1].size() < 4) continue;
    uint64_t perms = 0;
    if (tokens[1][0] == 'r') perms |= PROT_READ;
    if (tokens[1][1] == 'w') perms |= PROT_WRITE;
    if (tokens[1][2] == 'x') perms |= PROT_EXEC;

    uint64_t inode{};
    if (!absl::SimpleAtoi(tokens[4], &inode)) continue;

    std::string pathname;
    if (tokens.size() == 6) {
      absl::StripLeadingAsciiWhitespace(&tokens[5]);
      pathname = std::move(tokens[5]);
    }

    result.emplace_back(start, end, perms, offset, inode, std::move(pathname));
  }

  return result;
}

ErrorMessageOr<std::vector<LinuxMemoryMapping>> ReadAndParseMaps(pid_t pid) {
  OUTCOME_TRY(auto&& proc_pid_maps_content, ReadMaps(pid));
  return ParseMaps(proc_pid_maps_content);
}

}  // namespace orbit_module_utils
