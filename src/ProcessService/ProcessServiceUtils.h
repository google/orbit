// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_SERVICE_PROCESS_SERVICE_UTILS_H_
#define PROCESS_SERVICE_PROCESS_SERVICE_UTILS_H_

#include <stdint.h>

#include <ctime>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/services.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "ProcessService/CpuTime.h"

namespace orbit_process_service {

std::optional<orbit_process_service_internal::TotalCpuTime> GetCumulativeTotalCpuTime();
std::optional<orbit_process_service_internal::Jiffies> GetCumulativeCpuTimeFromProcess(pid_t pid);

// Searches on the instance for a symbols file. This can have 3 outcomes, an error, not found or
// success. In the success case it returns the symbol file path.
ErrorMessageOr<orbit_base::NotFoundOr<std::filesystem::path>> FindSymbolsFilePath(
    const orbit_grpc_protos::GetDebugInfoFileRequest& request);
bool ReadProcessMemory(uint32_t pid, uintptr_t address, void* buffer, uint64_t size,
                       uint64_t* num_bytes_read);

}  // namespace orbit_process_service

#endif  // PROCESS_SERVICE_PROCESS_SERVICE_UTILS_H_
