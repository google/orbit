// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitFunction.h"

#include <OrbitBase/Logging.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/match.h>

#include <map>

#include "Capture.h"
#include "Core.h"
#include "Log.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Params.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "Serialization.h"
#include "Utils.h"

Function::Function(std::string_view name, std::string_view pretty_name,
                   uint64_t address, uint64_t load_bias, uint64_t size,
                   std::string_view file, uint32_t line)
    : name_(name),
      pretty_name_(pretty_name),
      address_(address),
      load_bias_(load_bias),
      size_(size),
      file_(file),
      line_(line) {
  ResetStats();
  SetOrbitTypeFromName();
}

void Function::Select() {
  LOG("Selected %s at 0x%" PRIx64 " (address_=0x%" PRIx64
      ", load_bias_= 0x%" PRIx64 ", base_address=0x%" PRIx64 ")",
      pretty_name_, GetVirtualAddress(), address_, load_bias_,
      module_base_address_);
  Capture::GSelectedFunctionsMap[GetVirtualAddress()] = this;
}

void Function::UnSelect() {
  Capture::GSelectedFunctionsMap.erase(GetVirtualAddress());
}

bool Function::IsSelected() const {
  return Capture::GSelectedFunctionsMap.count(GetVirtualAddress()) > 0;
}

void Function::ResetStats() {
  if (stats_ == nullptr) {
    stats_ = std::make_shared<FunctionStats>();
  } else {
    stats_->Reset();
  }
}

void Function::UpdateStats(const Timer& timer) {
  if (stats_ != nullptr) {
    stats_->Update(timer);
  }
}

const absl::flat_hash_map<const char*, Function::OrbitType>&
Function::GetFunctionNameToOrbitTypeMap() {
  static absl::flat_hash_map<const char*, OrbitType> function_name_to_type_map{
      {"Start(", ORBIT_TIMER_START},
      {"Stop(", ORBIT_TIMER_STOP},
      {"StartAsync(", ORBIT_TIMER_START_ASYNC},
      {"StopAsync(", ORBIT_TIMER_STOP_ASYNC},
      {"TrackInt(", ORBIT_TRACK_INT},
      {"TrackInt64(", ORBIT_TRACK_INT_64},
      {"TrackUint(", ORBIT_TRACK_UINT},
      {"TrackUint64(", ORBIT_TRACK_UINT_64},
      {"TrackFloat(", ORBIT_TRACK_FLOAT},
      {"TrackDouble(", ORBIT_TRACK_DOUBLE},
      {"TrackFloatAsInt(", ORBIT_TRACK_FLOAT_AS_INT},
      {"TrackDoubleAsInt64(", ORBIT_TRACK_DOUBLE_AS_INT_64},
  };
  return function_name_to_type_map;
}

// Detect Orbit API functions by looking for special function names part of the
// orbit_api namespace. On a match, set the corresponding function type.
bool Function::SetOrbitTypeFromName() {
  const std::string& name = PrettyName();
  if (absl::StartsWith(name, "orbit_api::")) {
    for (auto& pair : GetFunctionNameToOrbitTypeMap()) {
      if (absl::StrContains(name, pair.first)) {
        SetOrbitType(pair.second);
        return true;
      }
    }
  }
  return false;
}

ORBIT_SERIALIZE(Function, 5) {
  ORBIT_NVP_VAL(4, name_);
  ORBIT_NVP_VAL(4, pretty_name_);
  ORBIT_NVP_VAL(4, loaded_module_path_);
  ORBIT_NVP_VAL(4, module_base_address_);
  ORBIT_NVP_VAL(4, address_);
  ORBIT_NVP_VAL(4, load_bias_);
  ORBIT_NVP_VAL(4, size_);
  ORBIT_NVP_VAL(4, file_);
  ORBIT_NVP_VAL(4, line_);
  ORBIT_NVP_VAL(4, stats_);
}

void Function::Print() {
  ORBIT_VIZV(address_);
  ORBIT_VIZV(file_);
  ORBIT_VIZV(line_);
  ORBIT_VIZV(IsSelected());
}
