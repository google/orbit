// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureData.h"

#include <memory>

#include "OrbitBase/Profiling.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ModuleData.h"
#include "process.pb.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::LinuxAddressInfo;

const FunctionStats& CaptureData::GetFunctionStatsOrDefault(const FunctionInfo& function) const {
  static const FunctionStats kDefaultFunctionStats;
  uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(function);
  auto function_stats_it = functions_stats_.find(absolute_address);
  if (function_stats_it == functions_stats_.end()) {
    return kDefaultFunctionStats;
  }
  return function_stats_it->second;
}

void CaptureData::UpdateFunctionStats(const FunctionInfo& function, uint64_t elapsed_nanos) {
  const uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(function);
  FunctionStats& stats = functions_stats_[absolute_address];
  stats.set_count(stats.count() + 1);
  stats.set_total_time_ns(stats.total_time_ns() + elapsed_nanos);
  stats.set_average_time_ns(stats.total_time_ns() / stats.count());

  if (elapsed_nanos > stats.max_ns()) {
    stats.set_max_ns(elapsed_nanos);
  }

  if (stats.min_ns() == 0 || elapsed_nanos < stats.min_ns()) {
    stats.set_min_ns(elapsed_nanos);
  }
}

const FunctionInfo* CaptureData::GetSelectedFunction(uint64_t function_address) const {
  auto selected_functions_it = selected_functions_.find(function_address);
  if (selected_functions_it == selected_functions_.end()) {
    return nullptr;
  }
  return &selected_functions_it->second;
}

const LinuxAddressInfo* CaptureData::GetAddressInfo(uint64_t absolute_address) const {
  auto address_info_it = address_infos_.find(absolute_address);
  if (address_info_it == address_infos_.end()) {
    return nullptr;
  }
  return &address_info_it->second;
}

void CaptureData::InsertAddressInfo(LinuxAddressInfo address_info) {
  const uint64_t absolute_address = address_info.absolute_address();
  const uint64_t absolute_function_address = absolute_address - address_info.offset_in_function();
  // Ensure we know the symbols also for the resolved function address;
  if (!address_infos_.contains(absolute_function_address)) {
    LinuxAddressInfo function_info;
    function_info.CopyFrom(address_info);
    function_info.set_absolute_address(absolute_function_address);
    function_info.set_offset_in_function(0);
    address_infos_.emplace(absolute_function_address, function_info);
  }
  address_infos_.emplace(absolute_address, std::move(address_info));
}

const std::string CaptureData::kUnknownFunctionOrModuleName{"???"};

const std::string& CaptureData::GetFunctionNameByAddress(uint64_t absolute_address) const {
  const FunctionInfo* function = FindFunctionByAddress(absolute_address, false);
  if (function != nullptr) {
    return FunctionUtils::GetDisplayName(*function);
  }
  const auto address_info_it = address_infos_.find(absolute_address);
  if (address_info_it == address_infos_.end()) {
    return kUnknownFunctionOrModuleName;
  }
  const LinuxAddressInfo& address_info = address_info_it->second;
  const std::string& function_name = address_info.function_name();
  if (function_name.empty()) {
    return kUnknownFunctionOrModuleName;
  }
  return function_name;
}

const std::string& CaptureData::GetModulePathByAddress(uint64_t absolute_address) const {
  const ModuleData* module_data = FindModuleByAddress(absolute_address);
  if (module_data != nullptr) {
    return module_data->file_path();
  }
  const auto address_info_it = address_infos_.find(absolute_address);
  if (address_info_it == address_infos_.end()) {
    return kUnknownFunctionOrModuleName;
  }
  const LinuxAddressInfo& address_info = address_info_it->second;
  const std::string& module_path = address_info.module_path();
  if (module_path.empty()) {
    return kUnknownFunctionOrModuleName;
  }
  return module_path;
}

const FunctionInfo* CaptureData::FindFunctionByAddress(uint64_t absolute_address,
                                                       bool is_exact) const {
  // TODO(antonrohr) Maybe do a CHECK(process_ != nullptr) here instead of returning -1. Currently
  // CaptureSerializerTest is the only scenario where this is actually needed.
  if (process_ == nullptr) {
    return nullptr;
  }
  const auto result = process_->FindModuleByAddress(absolute_address);
  if (!result) return nullptr;
  const std::string& module_path = result.value().first;
  const uint64_t module_base_address = result.value().second;

  const ModuleData* module = module_map_.at(module_path);

  const uint64_t relative_address = absolute_address - module_base_address;
  return module->FindFunctionByRelativeAddress(relative_address, is_exact);
}

[[nodiscard]] ModuleData* CaptureData::FindModuleByAddress(uint64_t absolute_address) const {
  // TODO(antonrohr) Maybe do a CHECK(process_ != nullptr) here instead of returning -1. Currently
  // CaptureSerializerTest is the only scenario where this is actually needed.
  if (process_ == nullptr) {
    return nullptr;
  }
  const auto result = process_->FindModuleByAddress(absolute_address);
  if (!result) return nullptr;
  return module_map_.at(result.value().first);
}

int32_t CaptureData::process_id() const {
  // TODO(antonrohr) Maybe do a CHECK(process_ != nullptr) here instead of returning -1. Currently
  // CaptureSerializerTest is the only scenario where this is actually needed.
  return process_ != nullptr ? process_->pid() : -1;
}

const std::string CaptureData::process_name() const {
  // TODO(antonrohr) Maybe do a CHECK(process_ != nullptr) here instead of returning an empty
  // string. Currently CaptureSerializerTest is the only scenario where this is actually needed.
  return process_ != nullptr ? process_->name() : "";
}