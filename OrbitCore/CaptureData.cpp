// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureData.h"

#include "Profiling.h"

using orbit_client_protos::FunctionStats;

orbit_client_protos::LinuxAddressInfo* CaptureData::GetAddressInfo(
    uint64_t address) {
  auto address_info_it = address_infos_.find(address);
  if (address_info_it == address_infos_.end()) {
    return nullptr;
  }
  return &address_info_it->second;
}

const FunctionStats& CaptureData::GetFunctionStatsOrDefault(
    uint64_t function_address) {
  static const FunctionStats kDefaultFunctionStats;
  auto function_stats_it = functions_stats_.find(function_address);
  if (function_stats_it == functions_stats_.end()) {
    return kDefaultFunctionStats;
  }
  return function_stats_it->second;
}

void CaptureData::UpdateFunctionStats(
    orbit_client_protos::FunctionInfo* func,
    const orbit_client_protos::TimerInfo& timer_info) {
  const uint64_t function_address = func->address();
  FunctionStats& stats = functions_stats_[function_address];
  stats.set_count(stats.count() + 1);
  uint64_t elapsed_nanos =
      TicksToNanoseconds(timer_info.start(), timer_info.end());
  stats.set_total_time_ns(stats.total_time_ns() + elapsed_nanos);
  stats.set_average_time_ns(stats.total_time_ns() / stats.count());

  if (elapsed_nanos > stats.max_ns()) {
    stats.set_max_ns(elapsed_nanos);
  }

  if (stats.min_ns() == 0 || elapsed_nanos < stats.min_ns()) {
    stats.set_min_ns(elapsed_nanos);
  }
}
