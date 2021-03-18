// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessUtils.h"

#include "OrbitBase/Logging.h"
#include "ServiceUtils.h"

namespace orbit_service {

ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModulesFromProcMaps(int32_t pid) {
  const auto proc_maps_data = orbit_elf_utils::ReadProcMapsFile(pid);
  if (proc_maps_data.has_error()) return proc_maps_data.error();

  const auto map_entries = orbit_elf_utils::ParseMaps(proc_maps_data.value());

  std::vector<orbit_grpc_protos::ModuleInfo> result;
  result.reserve(map_entries.size());

  for (const orbit_elf_utils::MapEntry& entry : map_entries) {
    if (!entry.is_executable) continue;

    ErrorMessageOr<orbit_grpc_protos::ModuleInfo> module_info_or_error = ErrorMessage{};

    if (entry.inode != 0) {
      module_info_or_error = CreateModuleFromFile(entry);
    } else {
      module_info_or_error = CreateModuleFromProcessMemory(pid, entry);
    }

    if (module_info_or_error.has_error()) {
      ERROR("Unable to create module: %s", module_info_or_error.error().message());
      continue;
    }

    result.emplace_back(std::move(module_info_or_error.value()));
  }

  return result;
}

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromProcessMemory(
    int32_t pid, const orbit_elf_utils::MapEntry& map_entry) {
  if (map_entry.end_address <= map_entry.start_address) {
    return ErrorMessage{
        absl::StrFormat("Invalid address range for module \"%s\".", map_entry.module_path)};
  }

  const auto size = map_entry.end_address - map_entry.start_address;

  // 3 MiB - It's more than enough for reading [vdso] which is typically around 8 KiB.
  if (size > 3 * 1024 * 1024) {
    return ErrorMessage{absl::StrFormat(
        "Module \"%s\" has a size of %d bytes (> 3MiB) and is too large to be read.",
        map_entry.module_path, size)};
  }

  std::string buffer(size, '\0');
  const auto result =
      utils::ReadProcessMemory(pid, map_entry.start_address, buffer.data(), buffer.size());

  if (result.has_error()) {
    return ErrorMessage{absl::StrFormat("Failed to read process memory for module \"%s\": %s",
                                        map_entry.module_path, result.error().message())};
  }

  return orbit_elf_utils::CreateModuleFromBuffer(map_entry, buffer);
}
}  // namespace orbit_service