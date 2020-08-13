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

namespace orbit_service::utils {
ErrorMessageOr<std::string> ExecuteCommand(const std::string& cmd);
ErrorMessageOr<std::vector<std::string>> ReadProcMaps(pid_t pid);
ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ListModules(int32_t pid);
ErrorMessageOr<std::unordered_map<pid_t, double>> GetCpuUtilization();
ErrorMessageOr<std::string> GetExecutablePath(int32_t pid);
ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name);
bool ReadProcessMemory(int32_t pid, uintptr_t address, void* buffer, uint64_t size,
                       uint64_t* num_bytes_read);
}  // namespace orbit_service::utils

#endif  // ORBIT_SERVICE_UTILS_H_