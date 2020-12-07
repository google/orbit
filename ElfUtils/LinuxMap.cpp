// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ElfUtils/LinuxMap.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_elf_utils {

using ElfUtils::ElfFile;
using orbit_grpc_protos::ModuleInfo;

static ErrorMessageOr<uint64_t> FileSize(const std::string& file_path) {
  struct stat stat_buf {};
  int ret = stat(file_path.c_str(), &stat_buf);
  if (ret != 0) {
    return ErrorMessage(absl::StrFormat("Unable to call stat with file \"%s\": %s", file_path,
                                        SafeStrerror(errno)));
  }
  return stat_buf.st_size;
}

ErrorMessageOr<std::vector<ModuleInfo>> ReadModules(int32_t pid) {
  std::filesystem::path proc_maps_path{absl::StrFormat("/proc/%d/maps", pid)};
  OUTCOME_TRY(proc_maps_data, orbit_base::ReadFileToString(proc_maps_path));
  return ParseMaps(proc_maps_data);
}

ErrorMessageOr<std::vector<ModuleInfo>> ParseMaps(std::string_view proc_maps_data) {
  struct AddressRange {
    uint64_t start_address;
    uint64_t end_address;
    bool is_executable;
  };

  const std::vector<std::string> proc_maps = absl::StrSplit(proc_maps_data, '\n');

  std::map<std::string, AddressRange> address_map;
  for (const std::string& line : proc_maps) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    // tokens[4] is the inode column. If inode equals 0, then the memory is not
    // mapped to a file (might be heap, stack or something else)
    if (tokens.size() != 6 || tokens[4] == "0") continue;

    const std::string& module_path = tokens[5];

    // This excludes mapped character or block devices.
    if (absl::StartsWith(module_path, "/dev/")) continue;

    std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;

    uint64_t start = std::stoull(addresses[0], nullptr, 16);
    uint64_t end = std::stoull(addresses[1], nullptr, 16);
    bool is_executable = tokens[1].size() == 4 && tokens[1][2] == 'x';

    auto iter = address_map.find(module_path);
    if (iter == address_map.end()) {
      address_map[module_path] = {start, end, is_executable};
    } else {
      AddressRange& address_range = iter->second;
      address_range.start_address = std::min(address_range.start_address, start);
      address_range.end_address = std::max(address_range.end_address, end);
      address_range.is_executable |= is_executable;
    }
  }

  std::vector<ModuleInfo> result;
  for (const auto& [module_path, address_range] : address_map) {
    // Filter out entries which are not executable
    if (!address_range.is_executable) continue;
    if (!std::filesystem::exists(module_path)) continue;
    ErrorMessageOr<uint64_t> file_size = FileSize(module_path);
    if (!file_size) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(module_path);
    if (!elf_file) {
      // TODO: Shouldn't this result in ErrorMessage?
      ERROR("Unable to load module \"%s\": %s - will ignore.", module_path,
            elf_file.error().message());
      continue;
    }

    ErrorMessageOr<uint64_t> load_bias = elf_file.value()->GetLoadBias();
    // Every loadable module contains a load bias.
    if (!load_bias) {
      ERROR("No load bias found for module %s", module_path.c_str());
      continue;
    }

    ModuleInfo module_info;
    module_info.set_name(std::filesystem::path{module_path}.filename());
    module_info.set_file_path(module_path);
    module_info.set_file_size(file_size.value());
    module_info.set_address_start(address_range.start_address);
    module_info.set_address_end(address_range.end_address);
    module_info.set_build_id(elf_file.value()->GetBuildId());
    module_info.set_load_bias(load_bias.value());

    result.push_back(module_info);
  }

  return result;
}

}  // namespace orbit_elf_utils