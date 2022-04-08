// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CaptureData.h"

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ScopeIdConstants.h"
#include "ObjectUtils/Address.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ProcessInfo;

namespace orbit_client_data {

CaptureData::CaptureData(const CaptureStarted& capture_started,
                         std::optional<std::filesystem::path> file_path,
                         absl::flat_hash_set<uint64_t> frame_track_function_ids,
                         DataSource data_source)
    : memory_sampling_period_ns_(capture_started.capture_options().memory_sampling_period_ns()),
      selection_callstack_data_(std::make_unique<CallstackData>()),
      frame_track_function_ids_{std::move(frame_track_function_ids)},
      file_path_{std::move(file_path)},
      scope_id_provider_(NameEqualityScopeIdProvider::Create(capture_started.capture_options())),
      thread_track_data_provider_(
          std::make_unique<ThreadTrackDataProvider>(data_source == DataSource::kLoadedCapture)) {
  ProcessInfo process_info;
  process_info.set_pid(capture_started.process_id());
  std::filesystem::path executable_path{capture_started.executable_path()};
  process_info.set_full_path(executable_path.string());
  process_info.set_name(executable_path.filename().string());
  process_info.set_is_64_bit(true);
  process_.SetProcessInfo(process_info);

  absl::flat_hash_map<uint64_t, FunctionInfo> instrumented_functions;

  for (const auto& instrumented_function :
       capture_started.capture_options().instrumented_functions()) {
    instrumented_functions_.insert_or_assign(instrumented_function.function_id(),
                                             instrumented_function);
  }
}

void CaptureData::ForEachThreadStateSliceIntersectingTimeRange(
    uint32_t thread_id, uint64_t min_timestamp, uint64_t max_timestamp,
    const std::function<void(const ThreadStateSliceInfo&)>& action) const {
  absl::MutexLock lock{&thread_state_slices_mutex_};
  auto tid_thread_state_slices_it = thread_state_slices_.find(thread_id);
  if (tid_thread_state_slices_it == thread_state_slices_.end()) {
    return;
  }

  const std::vector<ThreadStateSliceInfo>& tid_thread_state_slices =
      tid_thread_state_slices_it->second;
  auto slice_it = std::lower_bound(tid_thread_state_slices.begin(), tid_thread_state_slices.end(),
                                   min_timestamp,
                                   [](const ThreadStateSliceInfo& slice, uint64_t min_timestamp) {
                                     return slice.end_timestamp_ns() < min_timestamp;
                                   });
  while (slice_it != tid_thread_state_slices.end() &&
         slice_it->begin_timestamp_ns() < max_timestamp) {
    action(*slice_it);
    ++slice_it;
  }
}

const ScopeStats& CaptureData::GetScopeStatsOrDefault(uint64_t scope_id) const {
  static const ScopeStats kDefaultScopeStats;
  auto scope_stats_it = scope_stats_.find(scope_id);
  if (scope_stats_it == scope_stats_.end()) {
    return kDefaultScopeStats;
  }
  return scope_stats_it->second;
}

void CaptureData::UpdateScopeStats(const TimerInfo& timer_info) {
  const uint64_t scope_id = ProvideScopeId(timer_info);
  if (scope_id == kInvalidScopeId) return;

  ScopeStats& stats = scope_stats_[scope_id];
  const uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
  stats.UpdateStats(elapsed_nanos);
}

void CaptureData::AddScopeStats(uint64_t scope_id, ScopeStats stats) {
  scope_stats_.insert_or_assign(scope_id, stats);
}

void CaptureData::OnCaptureComplete() {
  thread_track_data_provider_->OnCaptureComplete();
  UpdateTimerDurations();
}

const InstrumentedFunction* CaptureData::GetInstrumentedFunctionById(uint64_t function_id) const {
  auto instrumented_functions_it = instrumented_functions_.find(function_id);
  if (instrumented_functions_it == instrumented_functions_.end()) {
    return nullptr;
  }
  return &instrumented_functions_it->second;
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
    LinuxAddressInfo function_info{absolute_function_address, /*offset_in_function=*/0,
                                   address_info.module_path(), address_info.function_name()};
    address_infos_.emplace(absolute_function_address, std::move(function_info));
  }
  address_infos_.emplace(absolute_address, std::move(address_info));
}

uint32_t CaptureData::process_id() const { return process_.pid(); }

std::string CaptureData::process_name() const { return process_.name(); }

void CaptureData::EnableFrameTrack(uint64_t instrumented_function_id) {
  if (frame_track_function_ids_.contains(instrumented_function_id)) {
    const auto* function = GetInstrumentedFunctionById(instrumented_function_id);
    ORBIT_CHECK(function != nullptr);
    ORBIT_LOG("Warning: Frame track for instrumented function \"%s\" is already enabled",
              function->function_name());
    return;
  }
  frame_track_function_ids_.insert(instrumented_function_id);
}

void CaptureData::DisableFrameTrack(uint64_t instrumented_function_id) {
  frame_track_function_ids_.erase(instrumented_function_id);
}

bool CaptureData::IsFrameTrackEnabled(uint64_t instrumented_function_id) const {
  return frame_track_function_ids_.contains(instrumented_function_id);
}

uint64_t CaptureData::ProvideScopeId(const orbit_client_protos::TimerInfo& timer_info) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->ProvideId(timer_info);
}

[[nodiscard]] std::vector<uint64_t> CaptureData::GetAllProvidedScopeIds() const {
  return scope_id_provider_->GetAllProvidedScopeIds();
}

const std::string& CaptureData::GetScopeName(uint64_t scope_id) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->GetScopeName(scope_id);
}

uint64_t CaptureData::FunctionIdToScopeId(uint64_t function_id) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->FunctionIdToScopeId(function_id);
}

const std::vector<uint64_t>* CaptureData::GetSortedTimerDurationsForScopeId(
    uint64_t scope_id) const {
  const auto it = scope_id_to_timer_durations_.find(scope_id);
  if (it == scope_id_to_timer_durations_.end()) return nullptr;
  return &it->second;
}

[[nodiscard]] std::vector<const TimerInfo*> CaptureData::GetAllScopeTimers(
    uint64_t min_tick, uint64_t max_tick) const {
  std::vector<const TimerInfo*> result;

  for (const uint32_t thread_id : GetThreadTrackDataProvider()->GetAllThreadIds()) {
    const std::vector<const TimerInfo*> thread_track_timers =
        GetThreadTrackDataProvider()->GetTimers(thread_id, min_tick, max_tick);
    result.insert(std::end(result), std::begin(thread_track_timers), std::end(thread_track_timers));
  }

  const std::vector<const TimerInfo*> async_scope_timers =
      timer_data_manager_.GetTimers(orbit_client_protos::TimerInfo::kApiScopeAsync);
  result.insert(std::end(result), std::begin(async_scope_timers), std::end(async_scope_timers));
  return result;
}

void CaptureData::UpdateTimerDurations() {
  ORBIT_SCOPE_FUNCTION;
  scope_id_to_timer_durations_.clear();
  for (const TimerInfo* timer : GetAllScopeTimers()) {
    const uint64_t scope_id = ProvideScopeId(*timer);

    if (scope_id == orbit_client_data::kInvalidScopeId) return;

    scope_id_to_timer_durations_[scope_id].push_back(timer->end() - timer->start());
  }

  for (auto& [id, timer_durations] : scope_id_to_timer_durations_) {
    std::sort(timer_durations.begin(), timer_durations.end());
  }
}

[[nodiscard]] std::vector<const TimerInfo*> CaptureData::GetTimersForScope(
    uint64_t scope_id, uint64_t min_tick, uint64_t max_tick) const {
  const std::vector<const TimerInfo*> all_timers = GetAllScopeTimers(min_tick, max_tick);
  std::vector<const TimerInfo*> result;
  std::copy_if(std::begin(all_timers), std::end(all_timers), std::back_inserter(result),
               [this, scope_id](const TimerInfo* timer) {
                 return scope_id_provider_->ProvideId(*timer) == scope_id;
               });
  return result;
}

}  // namespace orbit_client_data
