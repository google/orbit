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
#include "OrbitBase/Result.h"

namespace orbit_elf_utils {

using orbit_elf_utils::ElfFile;
using orbit_grpc_protos::ModuleInfo;

ErrorMessageOr<ModuleInfo> CreateModuleFromFile(const std::filesystem::path& module_path,
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
  module_info.set_is_virtual_module(false);

  return module_info;
}

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromFile(const MapEntry& map_entry) {
  return CreateModuleFromFile(map_entry.module_path, map_entry.start_address,
                              map_entry.end_address);
}

ErrorMessageOr<ModuleInfo> CreateModuleFromBuffer(std::string module_name, std::string_view buffer,
                                                  uint64_t start_address, uint64_t end_address) {
  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_or_error =
      ElfFile::CreateFromBuffer({}, buffer.data(), buffer.size());
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
  module_info.set_name(module_name);
  module_info.set_file_path(std::move(module_name));
  module_info.set_file_size(buffer.size());
  module_info.set_address_start(start_address);
  module_info.set_address_end(end_address);
  module_info.set_build_id(elf_file_or_error.value()->GetBuildId());
  module_info.set_load_bias(load_bias_or_error.value());
  module_info.set_is_virtual_module(true);

  return module_info;
}

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromBuffer(const MapEntry& map_entry,
                                                                     std::string_view buffer) {
  return CreateModuleFromBuffer(map_entry.module_path, buffer, map_entry.start_address,
                                map_entry.end_address);
}

ErrorMessageOr<std::string> ReadProcMapsFile(int32_t pid) {
  std::filesystem::path proc_maps_path{absl::StrFormat("/proc/%d/maps", pid)};
  return orbit_base::ReadFileToString(proc_maps_path);
}

ErrorMessageOr<std::vector<ModuleInfo>> ReadModules(int32_t pid) {
  const auto proc_maps_data = ReadProcMapsFile(pid);
  if (proc_maps_data.has_error()) return proc_maps_data.error();

  const auto map_entries = ParseMaps(proc_maps_data.value());

  std::vector<ModuleInfo> result;
  result.reserve(map_entries.size());

  for (const MapEntry& entry : map_entries) {
    if (entry.inode == 0 || !entry.is_executable) continue;

    auto module_info_or_error = CreateModuleFromFile(entry);
    if (module_info_or_error.has_error()) {
      ERROR("Unable to create module: %s", module_info_or_error.error().message());
      continue;
    }

    result.emplace_back(std::move(module_info_or_error.value()));
  }

  return result;
}

std::optional<MapEntry> ParseMapEntry(std::string_view proc_maps_line) {
  std::vector<std::string_view> tokens = absl::StrSplit(proc_maps_line, ' ', absl::SkipEmpty());
  if (tokens.size() != 6) return std::nullopt;

  std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
  if (addresses.size() != 2) return std::nullopt;

  MapEntry entry{};
  entry.module_path = tokens[5];
  entry.start_address = std::stoull(addresses[0], nullptr, 16);
  entry.end_address = std::stoull(addresses[1], nullptr, 16);
  entry.is_executable = tokens[1].size() == 4 && tokens[1][2] == 'x';

  // tokens[4] is the inode column. If inode equals 0, then the memory is not
  // mapped to a file (might be heap, stack or something else)
  entry.inode = std::stoull(std::string{tokens[4]}, nullptr, 10);
  return entry;
}

std::vector<MapEntry> ParseMaps(std::string_view proc_maps_data) {
  const std::vector<std::string> proc_maps = absl::StrSplit(proc_maps_data, '\n');

  std::map<std::string, MapEntry> address_map;
  for (const std::string& line : proc_maps) {
    const std::optional<MapEntry> maybe_map_entry = ParseMapEntry(line);

    if (!maybe_map_entry.has_value()) continue;
    const auto& map_entry = maybe_map_entry.value();

    auto iter = address_map.find(map_entry.module_path);
    if (iter == address_map.end()) {
      address_map[map_entry.module_path] = map_entry;
    } else {
      MapEntry& existing_map_entry = iter->second;
      existing_map_entry.start_address =
          std::min(existing_map_entry.start_address, map_entry.start_address);
      existing_map_entry.end_address =
          std::max(existing_map_entry.end_address, map_entry.end_address);
      existing_map_entry.is_executable |= map_entry.is_executable;
    }
  }

  std::vector<MapEntry> result;
  result.reserve(address_map.size());

  for (const auto& [module_path, map_entry] : address_map) {
    result.push_back(map_entry);
  }

  return result;
}

}  // namespace orbit_elf_utils