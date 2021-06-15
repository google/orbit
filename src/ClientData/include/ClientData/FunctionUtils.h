// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FUNCTION_UTILS_H_
#define CLIENT_DATA_FUNCTION_UTILS_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <memory>
#include <string>

#include "ClientData/ModuleData.h"
#include "ProcessData.h"
#include "capture_data.pb.h"
#include "symbol.pb.h"

namespace orbit_client_data {

namespace function_utils {

[[nodiscard]] inline const std::string& GetDisplayName(
    const orbit_client_protos::FunctionInfo& func) {
  return func.pretty_name().empty() ? func.name() : func.pretty_name();
}

[[nodiscard]] std::string GetLoadedModuleNameByPath(std::string_view module_path);
[[nodiscard]] std::string GetLoadedModuleName(const orbit_client_protos::FunctionInfo& func);
[[nodiscard]] uint64_t GetHash(const orbit_client_protos::FunctionInfo& func);
[[nodiscard]] uint64_t GetHash(std::string_view function_name);

// Calculates and returns the absolute address of the function.
[[nodiscard]] uint64_t Offset(const orbit_client_protos::FunctionInfo& func,
                              const ModuleData& module);

// This function should not be used, since it could return incomplete or invalid
// result in the case when one module is mapped two or more times. Try to find another way
// of solving the problem, for example by converting an absolute address to a module offset and
// then operating on that.
// TODO(b/191248550): Disassemble from file instead of doing it from process memory.
[[nodiscard, deprecated]] std::optional<uint64_t> GetAbsoluteAddress(
    const orbit_client_protos::FunctionInfo& func, const ProcessData& process,
    const ModuleData& module);

[[nodiscard]] bool IsOrbitFunctionFromType(
    const orbit_client_protos::FunctionInfo::OrbitType& type);

[[nodiscard]] bool IsOrbitFunctionFromName(const std::string& function_name);

[[nodiscard]] std::unique_ptr<orbit_client_protos::FunctionInfo> CreateFunctionInfo(
    const orbit_grpc_protos::SymbolInfo& symbol_info, const std::string& module_path,
    const std::string& module_build_id);

[[nodiscard]] const absl::flat_hash_map<std::string, orbit_client_protos::FunctionInfo::OrbitType>&
GetFunctionNameToOrbitTypeMap();
[[nodiscard]] orbit_client_protos::FunctionInfo::OrbitType GetOrbitTypeByName(
    const std::string& function_name);
void SetOrbitTypeFromName(orbit_client_protos::FunctionInfo* func);

}  // namespace function_utils

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FUNCTION_UTILS_H_
