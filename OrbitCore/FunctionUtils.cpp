// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionUtils.h"

#include <map>

#include "Capture.h"
#include "Log.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

namespace FunctionUtils {

std::string GetLoadedModuleName(const Function& func) {
  return Path::GetFileName(func.loaded_module_path());
}

uint64_t GetHash(const Function& func) {
  return StringHash(func.pretty_name());
}

uint64_t Offset(const Function& func) {
  return func.address() - func.load_bias();
}

bool IsOrbitFunc(const Function& func) {
  return func.orbit_type() != Function::OrbitType::NONE;
}

void Select(Function* func) {
  LOG("Selected %s at 0x%" PRIx64 " (address_=0x%" PRIx64
      ", load_bias_= 0x%" PRIx64 ", base_address=0x%" PRIx64 ")",
      func->pretty_name(), GetAbsoluteAddress(*func), func->address(),
      func->load_bias(), func->module_base_address());
  Capture::GSelectedFunctionsMap[GetAbsoluteAddress(*func)] = func;
}

void UnSelect(Function* func) {
  Capture::GSelectedFunctionsMap.erase(GetAbsoluteAddress(*func));
}

bool IsSelected(const Function& func) {
  return Capture::GSelectedFunctionsMap.count(GetAbsoluteAddress(func)) > 0;
}

void Print(const Function& func) {
  ORBIT_VIZV(func.address());
  ORBIT_VIZV(func.file());
  ORBIT_VIZV(func.line());
  ORBIT_VIZV(IsSelected(func));
}

const absl::flat_hash_map<const char*, Function::OrbitType>&
GetFunctionNameToOrbitTypeMap() {
  static absl::flat_hash_map<const char*, Function::OrbitType>
      function_name_to_type_map{
          {"Start(", Function::ORBIT_TIMER_START},
          {"Stop(", Function::ORBIT_TIMER_STOP},
          {"StartAsync(", Function::ORBIT_TIMER_START_ASYNC},
          {"StopAsync(", Function::ORBIT_TIMER_STOP_ASYNC},
          {"TrackInt(", Function::ORBIT_TRACK_INT},
          {"TrackInt64(", Function::ORBIT_TRACK_INT_64},
          {"TrackUint(", Function::ORBIT_TRACK_UINT},
          {"TrackUint64(", Function::ORBIT_TRACK_UINT_64},
          {"TrackFloat(", Function::ORBIT_TRACK_FLOAT},
          {"TrackDouble(", Function::ORBIT_TRACK_DOUBLE},
          {"TrackFloatAsInt(", Function::ORBIT_TRACK_FLOAT_AS_INT},
          {"TrackDoubleAsInt64(", Function::ORBIT_TRACK_DOUBLE_AS_INT_64},
      };
  return function_name_to_type_map;
}

// Detect Orbit API functions by looking for special function names part of the
// orbit_api namespace. On a match, set the corresponding function type.
bool SetOrbitTypeFromName(Function* func) {
  const std::string& name = GetDisplayName(*func);
  if (absl::StartsWith(name, "orbit_api::")) {
    for (auto& pair : GetFunctionNameToOrbitTypeMap()) {
      if (absl::StrContains(name, pair.first)) {
        func->set_orbit_type(pair.second);
        return true;
      }
    }
  }
  return false;
}

void UpdateStats(Function* func, const Timer& timer) {
  std::shared_ptr<FunctionStats> stats = func->stats();
  if (stats != nullptr) {
    stats->set_count(stats->count() + 1);
    double elapsedMillis = timer.ElapsedMillis();
    stats->set_total_time_ms(stats->total_time_ms() + elapsedMillis);
    stats->set_average_time_ms(stats->total_time_ms() / stats->count());

    if (elapsedMillis > stats->max_ms()) {
      stats->set_max_ms(elapsedMillis);
    }

    if (stats->min_ms() == 0 || elapsedMillis < stats->min_ms()) {
      stats->set_min_ms(elapsedMillis);
    }
  }
}

bool IsSelected(const SampledFunction& func) {
  return Capture::GSelectedFunctionsMap.count(func.m_Address) > 0;
}

}  // namespace FunctionUtils
