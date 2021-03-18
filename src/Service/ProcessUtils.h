// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_UTILS_H_
#define ORBIT_SERVICE_PROCESS_UTILS_H_

#include <stdint.h>

#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/Result.h"
#include "module.pb.h"

namespace orbit_service {

// This function is similar to orbit_elf_utils::ReadModules, but it also takes virtual modules
// into account.
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModulesFromProcMaps(int32_t pid);

// CreateModuleFromProcessMemory can create a ModuleInfo object by reading the ELF
// file from the mapped section in the target process. This is handy for modules which don't exist
// on the filesystem like the [vdso] module.
ErrorMessageOr<orbit_grpc_protos::ModuleInfo> CreateModuleFromProcessMemory(
    int32_t pid, const orbit_elf_utils::MapEntry& map_entry);

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PROCESS_UTILS_H_