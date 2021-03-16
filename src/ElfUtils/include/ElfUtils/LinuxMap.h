// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELF_UTILS_LINUX_MAP_H_
#define ELF_UTILS_LINUX_MAP_H_

#include <stdint.h>

#include <string_view>

#if defined(__linux)

#include <filesystem>
#include <vector>

#include "OrbitBase/Result.h"
#include "module.pb.h"

namespace orbit_elf_utils {

// MapEntry combines all the (needed) fields of a single line (mapping) from a Linux
// /proc/<pid>/maps file.
//
// It is the result type of `ParseMapEntry`.
struct MapEntry {
  std::string module_path;
  uint64_t start_address;
  uint64_t end_address;
  uint64_t inode;
  bool is_executable;
};

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromFile(
    const std::filesystem::path& module_path, uint64_t start_address, uint64_t end_address);
ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromFile(const MapEntry& map_entry);

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromBuffer(std::string module_name,
                                                                     std::string_view buffer,
                                                                     uint64_t start_address,
                                                                     uint64_t end_address);
ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromBuffer(const MapEntry& map_entry,
                                                                     std::string_view buffer);

ErrorMessageOr<std::string> ReadProcMapsFile(int32_t pid);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModules(int32_t pid);

std::optional<MapEntry> ParseMapEntry(std::string_view proc_maps_line);
std::vector<MapEntry> ParseMaps(std::string_view proc_maps_data);

}  // namespace orbit_elf_utils

#endif  // defined(__linux)
#endif  // ELF_UTILS_LINUX_MAP_H_
