// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionUtils.h"

#include <map>

#include "Capture.h"
#include "Log.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "Utils.h"

namespace FunctionUtils {

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::TimerInfo;

std::string GetLoadedModuleName(const FunctionInfo& func) {
  return Path::GetFileName(func.loaded_module_path());
}

uint64_t GetHash(const FunctionInfo& func) {
  return StringHash(func.pretty_name());
}

uint64_t Offset(const FunctionInfo& func) {
  return func.address() - func.load_bias();
}

bool IsOrbitFunc(const FunctionInfo& func) {
  return func.type() != FunctionInfo::kNone;
}

std::shared_ptr<FunctionInfo> CreateFunctionInfo(
    std::string name, std::string pretty_name, uint64_t address,
    uint64_t load_bias, uint64_t size, std::string file, uint32_t line,
    std::string loaded_module_path, uint64_t module_base_address) {
  std::shared_ptr<FunctionInfo> function_info =
      std::make_shared<FunctionInfo>();
  function_info->set_name(std::move(name));
  function_info->set_pretty_name(std::move(pretty_name));
  function_info->set_address(address);
  function_info->set_load_bias(load_bias);
  function_info->set_size(size);
  function_info->set_file(std::move(file));
  function_info->set_line(line);
  function_info->set_loaded_module_path(std::move(loaded_module_path));
  function_info->set_module_base_address(module_base_address);

  SetOrbitTypeFromName(function_info.get());
  return function_info;
}

void Select(FunctionInfo* func) {
  LOG("Selected %s at 0x%" PRIx64 " (address_=0x%" PRIx64
      ", load_bias_= 0x%" PRIx64 ", base_address=0x%" PRIx64 ")",
      func->pretty_name(), GetAbsoluteAddress(*func), func->address(),
      func->load_bias(), func->module_base_address());
  Capture::GSelectedFunctionsMap[GetAbsoluteAddress(*func)] = func;
}

void UnSelect(FunctionInfo* func) {
  Capture::GSelectedFunctionsMap.erase(GetAbsoluteAddress(*func));
}

bool IsSelected(const FunctionInfo& func) {
  return Capture::GSelectedFunctionsMap.count(GetAbsoluteAddress(func)) > 0;
}

void Print(const FunctionInfo& func) {
  ORBIT_VIZV(func.address());
  ORBIT_VIZV(func.file());
  ORBIT_VIZV(func.line());
  ORBIT_VIZV(IsSelected(func));
}

const absl::flat_hash_map<const char*, FunctionInfo::OrbitType>&
GetFunctionNameToOrbitTypeMap() {
  static absl::flat_hash_map<const char*, FunctionInfo::OrbitType>
      function_name_to_type_map{
          {"Start(", FunctionInfo::kOrbitTimerStart},
          {"Stop(", FunctionInfo::kOrbitTimerStop},
          {"StartAsync(", FunctionInfo::kOrbitTimerStartAsync},
          {"StopAsync(", FunctionInfo::kOrbitTimerStopAsync},
          {"TrackInt(", FunctionInfo::kOrbitTrackInt},
          {"TrackInt64(", FunctionInfo::kOrbitTrackInt64},
          {"TrackUint(", FunctionInfo::kOrbitTrackUint},
          {"TrackUint64(", FunctionInfo::kOrbitTrackUint64},
          {"TrackFloat(", FunctionInfo::kOrbitTrackFloat},
          {"TrackDouble(", FunctionInfo::kOrbitTrackDouble},
          {"TrackFloatAsInt(", FunctionInfo::kOrbitTrackFloatAsInt},
          {"TrackDoubleAsInt64(", FunctionInfo::kOrbitTrackDoubleAsInt64},
      };
  return function_name_to_type_map;
}

// Detect Orbit API functions by looking for special function names part of the
// orbit_api namespace. On a match, set the corresponding function type.
bool SetOrbitTypeFromName(FunctionInfo* func) {
  const std::string& name = GetDisplayName(*func);
  if (absl::StartsWith(name, "orbit_api::")) {
    for (auto& pair : GetFunctionNameToOrbitTypeMap()) {
      if (absl::StrContains(name, pair.first)) {
        func->set_type(pair.second);
        return true;
      }
    }
  }
  return false;
}

bool IsSelected(const SampledFunction& func) {
  return Capture::GSelectedFunctionsMap.count(func.address) > 0;
}

}  // namespace FunctionUtils
