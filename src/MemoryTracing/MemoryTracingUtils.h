// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_TRACING_UTILS_H_
#define MEMORY_TRACING_MEMORY_TRACING_UTILS_H_

#include <string_view>

#include "OrbitBase/Result.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {

[[nodiscard]] orbit_grpc_protos::SystemMemoryUsage CreateAndInitializeSystemMemoryUsage();
// All the following GetValues methods only update the input memory proto when the input file
// content format is correct. Before calling the GetValues methods, the memory proto fields that
// need to be updated should be initialized to kMissingInfo, (e.g., with the CreateAndInitialize
// method).
void GetValuesFromMemInfo(std::string_view meminfo_content,
                          orbit_grpc_protos::SystemMemoryUsage* system_memory_usage);
void GetValuesFromVmStat(std::string_view vmstat_content,
                         orbit_grpc_protos::SystemMemoryUsage* system_memory_usage);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::SystemMemoryUsage> GetSystemMemoryUsage() noexcept;

[[nodiscard]] orbit_grpc_protos::ProcessMemoryUsage CreateAndInitializeProcessMemoryUsage();
void GetValuesFromProcessStat(std::string_view stat_content,
                              orbit_grpc_protos::ProcessMemoryUsage* process_memory_usage);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ProcessMemoryUsage> GetProcessMemoryUsage(
    uint32_t pid) noexcept;

[[nodiscard]] orbit_grpc_protos::CGroupMemoryUsage CreateAndInitializeCGroupMemoryUsage();
[[nodiscard]] std::string GetProcessMemoryCGroupName(std::string_view cgroup_content);
void GetCGroupMemoryLimitInBytes(std::string_view memory_limit_in_bytes_content,
                                 orbit_grpc_protos::CGroupMemoryUsage* cgroup_memory_usage);
void GetValuesFromCGroupMemoryStat(std::string_view memory_stat_content,
                                   orbit_grpc_protos::CGroupMemoryUsage* cgroup_memory_usage);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::CGroupMemoryUsage> GetCGroupMemoryUsage(
    uint32_t pid) noexcept;

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_TRACING_UTILS_H_