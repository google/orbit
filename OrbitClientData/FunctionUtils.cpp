// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/FunctionUtils.h"

#include <map>

#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/match.h"
#include "xxhash.h"

namespace {
uint64_t StringHash(const std::string& string) {
  return XXH64(string.data(), string.size(), 0xBADDCAFEDEAD10CC);
}
}  // namespace

namespace FunctionUtils {

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::SymbolInfo;

std::string GetLoadedModuleName(const FunctionInfo& func) {
  return std::filesystem::path(func.loaded_module_path()).filename().string();
}

uint64_t GetHash(const FunctionInfo& func) { return StringHash(func.pretty_name()); }

uint64_t Offset(const FunctionInfo& func, const ModuleData& module) {
  return func.address() - module.load_bias();
}

bool IsOrbitFunc(const FunctionInfo& func) { return func.orbit_type() != FunctionInfo::kNone; }

std::unique_ptr<FunctionInfo> CreateFunctionInfo(const SymbolInfo& symbol_info,
                                                 const std::string& module_path) {
  auto function_info = std::make_unique<FunctionInfo>();

  function_info->set_name(symbol_info.name());
  function_info->set_pretty_name(symbol_info.demangled_name());
  function_info->set_address(symbol_info.address());
  function_info->set_size(symbol_info.size());
  function_info->set_file("");
  function_info->set_line(0);
  function_info->set_loaded_module_path(module_path);

  SetOrbitTypeFromName(function_info.get());
  return function_info;
}

#define STUB_PARAMS \
  "(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)"

const absl::flat_hash_map<const char*, FunctionInfo::OrbitType>& GetFunctionNameToOrbitTypeMap() {
  static absl::flat_hash_map<const char*, FunctionInfo::OrbitType> function_name_to_type_map{
      {"orbit_api::Start" STUB_PARAMS, FunctionInfo::kOrbitTimerStart},
      {"orbit_api::Stop" STUB_PARAMS, FunctionInfo::kOrbitTimerStop},
      {"orbit_api::StartAsync" STUB_PARAMS, FunctionInfo::kOrbitTimerStartAsync},
      {"orbit_api::StopAsync" STUB_PARAMS, FunctionInfo::kOrbitTimerStopAsync},
      {"orbit_api::TrackValue" STUB_PARAMS, FunctionInfo::kOrbitTrackValue}};
  return function_name_to_type_map;
}

// Detect Orbit API functions by looking for special function names part of the
// orbit_api namespace. On a match, set the corresponding function type.
bool SetOrbitTypeFromName(FunctionInfo* func) {
  const std::string& name = GetDisplayName(*func);
  if (absl::StartsWith(name, "orbit_api::")) {
    for (auto& pair : GetFunctionNameToOrbitTypeMap()) {
      if (absl::StrContains(name, pair.first)) {
        LOG("Found orbit_api function: %s", name);
        func->set_orbit_type(pair.second);
        return true;
      }
    }
  }
  return false;
}

}  // namespace FunctionUtils
