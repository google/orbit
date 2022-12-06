// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULE_UTILS_READ_LINUX_MODULES_H_
#define MODULE_UTILS_READ_LINUX_MODULES_H_

#ifdef __linux

#include <absl/types/span.h>
#include <stdint.h>
#include <unistd.h>

#include <filesystem>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"
#include "ReadLinuxMaps.h"

namespace orbit_module_utils {

ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModule(const std::filesystem::path& module_path,
                                                           uint64_t start_address,
                                                           uint64_t end_address);

ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModules(pid_t pid);

[[nodiscard]] std::vector<orbit_grpc_protos::ModuleInfo> ReadModulesFromMaps(
    absl::Span<const LinuxMemoryMapping> maps);

}  // namespace orbit_module_utils

#endif  // __linux

#endif  // MODULE_UTILS_READ_LINUX_MODULES_H_
