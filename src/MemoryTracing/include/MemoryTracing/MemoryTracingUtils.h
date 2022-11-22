// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_TRACING_UTILS_H_
#define MEMORY_TRACING_MEMORY_TRACING_UTILS_H_

#include <stdint.h>
#include <sys/types.h>

#include <string>
#include <string_view>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_memory_tracing {

[[nodiscard]] orbit_grpc_protos::SystemMemoryUsage CreateAndInitializeSystemMemoryUsage();
// All the following Update methods only update the input memory proto when the input file
// content format is correct. Before calling the Update methods, the memory proto fields that
// need to be updated should be initialized to kMissingInfo, (e.g., with the CreateAndInitialize
// method).
[[nodiscard]] ErrorMessageOr<void> UpdateSystemMemoryUsageFromMemInfo(
    std::string_view meminfo_content, orbit_grpc_protos::SystemMemoryUsage* system_memory_usage);
[[nodiscard]] ErrorMessageOr<void> UpdateSystemMemoryUsageFromVmStat(
    std::string_view vmstat_content, orbit_grpc_protos::SystemMemoryUsage* system_memory_usage);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::SystemMemoryUsage> GetSystemMemoryUsage();

[[nodiscard]] orbit_grpc_protos::ProcessMemoryUsage CreateAndInitializeProcessMemoryUsage();
[[nodiscard]] ErrorMessageOr<void> UpdateProcessMemoryUsageFromProcessStat(
    std::string_view stat_content, orbit_grpc_protos::ProcessMemoryUsage* process_memory_usage);
[[nodiscard]] ErrorMessageOr<int64_t> ExtractRssAnonFromProcessStatus(
    std::string_view status_content);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ProcessMemoryUsage> GetProcessMemoryUsage(
    pid_t pid);

[[nodiscard]] orbit_grpc_protos::CGroupMemoryUsage CreateAndInitializeCGroupMemoryUsage();
[[nodiscard]] std::string GetProcessMemoryCGroupName(std::string_view cgroup_content);
[[nodiscard]] ErrorMessageOr<void> UpdateCGroupMemoryUsageFromMemoryLimitInBytes(
    std::string_view memory_limit_in_bytes_content,
    orbit_grpc_protos::CGroupMemoryUsage* cgroup_memory_usage);
[[nodiscard]] ErrorMessageOr<void> UpdateCGroupMemoryUsageFromMemoryStat(
    std::string_view memory_stat_content,
    orbit_grpc_protos::CGroupMemoryUsage* cgroup_memory_usage);
[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::CGroupMemoryUsage> GetCGroupMemoryUsage(pid_t pid);

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_TRACING_UTILS_H_
