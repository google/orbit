// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_FUNCTION_UTILS_H_
#define ORBIT_CLIENT_DATA_FUNCTION_UTILS_H_

#include <cstdint>
#include <string>

#include "ProcessData.h"
#include "capture_data.pb.h"
#include "symbol.pb.h"

namespace FunctionUtils {

[[nodiscard]] inline const std::string& GetDisplayName(
    const orbit_client_protos::FunctionInfo& func) {
  return func.pretty_name().empty() ? func.name() : func.pretty_name();
}

[[nodiscard]] std::string GetLoadedModuleName(const orbit_client_protos::FunctionInfo& func);
[[nodiscard]] uint64_t GetHash(const orbit_client_protos::FunctionInfo& func);

// Calculates and returns the absolute address of the function.
[[nodiscard]] uint64_t Offset(const orbit_client_protos::FunctionInfo& func,
                              const ModuleData& module);
[[nodiscard]] inline uint64_t GetAbsoluteAddress(const orbit_client_protos::FunctionInfo& func,
                                                 const ProcessData& process,
                                                 const ModuleData& module) {
  return func.address() + process.GetModuleBaseAddress(func.loaded_module_path()) -
         module.load_bias();
}

[[nodiscard]] bool IsOrbitFunc(const orbit_client_protos::FunctionInfo& func);

[[nodiscard]] std::unique_ptr<orbit_client_protos::FunctionInfo> CreateFunctionInfo(
    const orbit_grpc_protos::SymbolInfo& symbol_info, const std::string& module_path);

[[nodiscard]] const absl::flat_hash_map<std::string, orbit_client_protos::FunctionInfo::OrbitType>&
GetFunctionNameToOrbitTypeMap();
bool SetOrbitTypeFromName(orbit_client_protos::FunctionInfo* func);

}  // namespace FunctionUtils

#endif  // ORBIT_CORE_FUNCTION_UTILS_H_
