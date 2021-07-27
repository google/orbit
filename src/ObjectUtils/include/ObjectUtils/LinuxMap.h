// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_LINUX_MAP_H_
#define OBJECT_UTILS_LINUX_MAP_H_

#include <stdint.h>

#include <string_view>

#if defined(__linux)

#include <filesystem>
#include <vector>

#include "OrbitBase/Result.h"
#include "module.pb.h"

namespace orbit_object_utils {

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModule(const std::filesystem::path& module_path,
                                                           uint64_t start_address,
                                                           uint64_t end_address);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModules(int32_t pid);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ParseMaps(
    std::string_view proc_maps_data);

uint64_t SymbolVirtualAddressToAbsoluteAddress(uint64_t symbol_address,
                                               uint64_t module_base_address,
                                               uint64_t module_load_bias,
                                               uint64_t module_executable_section_offset);

}  // namespace orbit_object_utils

#endif  // defined(__linux)
#endif  // OBJECT_UTILS_LINUX_MAP_H_
