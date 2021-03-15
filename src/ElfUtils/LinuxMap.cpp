// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ElfUtils/LinuxMap.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_elf_utils {

using orbit_elf_utils::ElfFile;
using orbit_grpc_protos::ModuleInfo;

ErrorMessageOr<ModuleInfo> CreateModule(const std::filesystem::path& module_path,
                                        uint64_t start_address, uint64_t end_address) {
  // This excludes mapped character or block devices.
  if (absl::StartsWith(module_path.string(), "/dev/")) {
    return ErrorMessage(absl::StrFormat(
        "The module \"%s\" is a character or block device (is in /dev/)", module_path));
  }

  if (!std::filesystem::exists(module_path)) {
    return ErrorMessage(absl::StrFormat("The module file \"%s\" does not exist", module_path));
  }
  std::error_code error;
  uint64_t file_size = std::filesystem::file_size(module_path, error);
  if (error) {
    return ErrorMessage(
        absl::StrFormat("Unable to get size of \"%s\": %s", module_path, error.message()));
  }

  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_or_error = ElfFile::Create(module_path);
  if (elf_file_or_error.has_error()) {
    return ErrorMessage(
        absl::StrFormat("Unable to load module: %s", elf_file_or_error.error().message()));
  }

  ErrorMessageOr<uint64_t> load_bias_or_error = elf_file_or_error.value()->GetLoadBias();
  // Every loadable module contains a load bias.
  if (load_bias_or_error.has_error()) {
    return load_bias_or_error.error();
  }

  ModuleInfo module_info;
  std::string soname = elf_file_or_error.value()->GetSoname();
  module_info.set_name(soname.empty() ? std::filesystem::path{module_path}.filename().string()
                                      : soname);
  module_info.set_file_path(module_path);
  module_info.set_file_size(file_size);
  module_info.set_address_start(start_address);
  module_info.set_address_end(end_address);
  module_info.set_build_id(elf_file_or_error.value()->GetBuildId());
  module_info.set_load_bias(load_bias_or_error.value());

  return module_info;
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

    ErrorMessageOr<ModuleInfo> module_info_or_error =
        CreateModule(module_path, address_range.start_address, address_range.end_address);
    if (module_info_or_error.has_error()) {
      ERROR("Unable to create module: %s", module_info_or_error.error().message());
      continue;
    }

    result.push_back(std::move(module_info_or_error.value()));
  }

  return result;
}

}  // namespace orbit_elf_utils