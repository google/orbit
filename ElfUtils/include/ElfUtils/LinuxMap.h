// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELF_UTILS_LINUX_MAP_H_
#define ELF_UTILS_LINUX_MAP_H_
#if defined(__linux)

#include <filesystem>
#include <vector>

#include "OrbitBase/Result.h"
#include "module.pb.h"

namespace orbit_elf_utils {

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModule(const std::filesystem::path& module_path,
                                                           uint64_t start_address,
                                                           uint64_t end_address);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModules(int32_t pid);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ParseMaps(
    std::string_view proc_maps_data);

}  // namespace orbit_elf_utils

#endif  // defined(__linux)
#endif  // ELF_UTILS_LINUX_MAP_H_
