// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_SERVICE_UTILS_H_
#define ORBIT_SERVICE_SERVICE_UTILS_H_

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

#include "OrbitBase/Result.h"
#include "module.pb.h"
#include "services.pb.h"
#include "tracepoint.pb.h"

namespace orbit_service::utils {
ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints();

// In the linux world, Jiffies is a global counter which increments on tick (caused by a CPU timer
// interrupt). This struct is a poor man's strong type to ensure that this measure is not mistakenly
// interpreted as nanoseconds.
struct Jiffies {
  uint64_t value;
};

struct TotalCpuTime {
  // jiffies is the sum over all cycles executed on all cores.
  Jiffies jiffies;

  // cpus is the number of (logical) cores available and accumulated in jiffies.
  size_t cpus;
};

std::optional<TotalCpuTime> GetCumulativeTotalCpuTime() noexcept;
std::optional<Jiffies> GetCumulativeCpuTimeFromProcess(pid_t pid) noexcept;

ErrorMessageOr<std::filesystem::path> FindSymbolsFilePath(
    const orbit_grpc_protos::GetDebugInfoFileRequest& request,
    const std::vector<std::filesystem::path>& search_directories = {
        "/home/cloudcast/", "/home/cloudcast/debug_symbols/", "/mnt/developer/",
        "/mnt/developer/debug_symbols/", "/srv/game/assets/", "/srv/game/assets/debug_symbols/",
        "/home/cloudcast/symbols", "/mnt/developer/symbols", "/srv/game/assets/symbols"}) noexcept;
bool ReadProcessMemory(uint32_t pid, uintptr_t address, void* buffer, uint64_t size,
                       uint64_t* num_bytes_read) noexcept;
}  // namespace orbit_service::utils

#endif  // ORBIT_SERVICE_SERVICE_UTILS_H_
