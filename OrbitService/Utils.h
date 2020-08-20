// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_UTILS_H_
#define ORBIT_SERVICE_UTILS_H_

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "OrbitBase/Result.h"
#include "module.pb.h"
#include "tracepoint.pb.h"

namespace orbit_service::utils {
using Path = std::filesystem::path;
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ReadModules(int32_t pid);
ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints();
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ParseMaps(
    std::string_view proc_maps_data);
ErrorMessageOr<std::unordered_map<pid_t, double>> GetCpuUtilization();
ErrorMessageOr<std::unordered_map<pid_t, double>> GetCpuUtilization(std::string_view top_data);
ErrorMessageOr<Path> GetExecutablePath(int32_t pid);
ErrorMessageOr<std::string> ReadFileToString(const Path& file_name);
ErrorMessageOr<Path> FindSymbolsFilePath(const Path& module_path,
                                         const std::vector<Path>& search_directories = {
                                             "/home/cloudcast/", "/home/cloudcast/debug_symbols/",
                                             "/mnt/developer/", "/mnt/developer/debug_symbols/",
                                             "/srv/game/assets/",
                                             "/srv/game/assets/debug_symbols/"});
bool ReadProcessMemory(int32_t pid, uintptr_t address, void* buffer, uint64_t size,
                       uint64_t* num_bytes_read);
}  // namespace orbit_service::utils

#endif  // ORBIT_SERVICE_UTILS_H_