// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CaptureData.h"

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "ClientData/FastRenderingUtils.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/ScopeStatsCollection.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Typedef.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ProcessInfo;
using orbit_symbol_provider::ModuleIdentifier;

namespace orbit_client_data {

CaptureData::CaptureData(CaptureStarted capture_started,
                         std::optional<std::filesystem::path> file_path,
                         absl::flat_hash_set<uint64_t> frame_track_function_ids,
                         DataSource data_source)
    : capture_started_(std::move(capture_started)),
      selection_callstack_data_(std::make_unique<CallstackData>()),
      frame_track_function_ids_{std::move(frame_track_function_ids)},
      file_path_{std::move(file_path)},
      scope_id_provider_(NameEqualityScopeIdProvider::Create(capture_started_.capture_options())),
      thread_track_data_provider_(
          std::make_unique<ThreadTrackDataProvider>(data_source == DataSource::kLoadedCapture)),
      all_scopes_(std::make_shared<ScopeStatsCollection>()) {
  ProcessInfo process_info;
  process_info.set_pid(capture_started_.process_id());
  std::filesystem::path executable_path{capture_started_.executable_path()};
  process_info.set_full_path(executable_path.string());
  process_info.set_name(executable_path.filename().string());
  process_info.set_is_64_bit(true);
  process_.SetProcessInfo(process_info);

  for (const auto& instrumented_function :
       capture_started_.capture_options().instrumented_functions()) {
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

void CaptureData::ForEachThreadStateSliceIntersectingTimeRangeDiscretized(
    uint32_t thread_id, uint64_t min_timestamp, uint64_t max_timestamp, uint32_t resolution,
    const std::function<void(const ThreadStateSliceInfo&)>& action) const {
  absl::MutexLock lock{&thread_state_slices_mutex_};
  auto tid_thread_state_slices_it = thread_state_slices_.find(thread_id);
  if (tid_thread_state_slices_it == thread_state_slices_.end()) {
    return;
  }

  const std::vector<ThreadStateSliceInfo>& tid_thread_state_slices =
      tid_thread_state_slices_it->second;

  auto thread_state_slices_lower_bound = [&](uint64_t timestamp) {
    return std::lower_bound(tid_thread_state_slices.begin(), tid_thread_state_slices.end(),
                            timestamp, [](const ThreadStateSliceInfo& slice, uint64_t timestamp) {
                              return timestamp >= slice.end_timestamp_ns();
                            });
  };

  uint64_t current_timestamp = min_timestamp;
  auto slice_it = thread_state_slices_lower_bound(current_timestamp);
  while (slice_it != tid_thread_state_slices.end() &&
         slice_it->begin_timestamp_ns() < max_timestamp) {
    action(*slice_it);
    current_timestamp = GetNextPixelBoundaryTimeNs(slice_it->end_timestamp_ns(), resolution,
                                                   min_timestamp, max_timestamp);
    slice_it = thread_state_slices_lower_bound(current_timestamp);
  }
}

const ScopeStats& CaptureData::GetScopeStatsOrDefault(ScopeId scope_id) const {
  return all_scopes_->GetScopeStatsOrDefault(scope_id);
}

void CaptureData::UpdateScopeStats(const TimerInfo& timer_info) {
  const std::optional<ScopeId> scope_id = ProvideScopeId(timer_info);
  if (!scope_id.has_value()) return;

  all_scopes_->UpdateScopeStats(scope_id.value(), timer_info);
}

void CaptureData::AddScopeStats(ScopeId scope_id, ScopeStats stats) {
  all_scopes_->SetScopeStats(scope_id, stats);
}

void CaptureData::OnCaptureComplete() {
  thread_track_data_provider_->OnCaptureComplete();
  all_scopes_->OnCaptureComplete();
}

void CaptureData::FilterBrokenCallstacks() {
  std::map<uint64_t, uint64_t> absolute_address_to_size_of_functions_to_stop_unwinding_at{};
  for (const orbit_grpc_protos::FunctionToStopUnwindingAt& function_to_stop_unwinding_at :
       capture_started_.capture_options().functions_to_stop_unwinding_at()) {
    auto [unused_it, inserted] =
        absolute_address_to_size_of_functions_to_stop_unwinding_at.insert_or_assign(
            function_to_stop_unwinding_at.absolute_address(), function_to_stop_unwinding_at.size());
    ORBIT_CHECK(inserted);
  }
  callstack_data_.UpdateCallstackTypeBasedOnMajorityStart(
      absolute_address_to_size_of_functions_to_stop_unwinding_at);
}

const FunctionInfo* CaptureData::GetFunctionInfoById(uint64_t function_id) const {
  ORBIT_CHECK(scope_id_provider_);
  auto scope_id = scope_id_provider_->FunctionIdToScopeId(function_id);
  if (scope_id.has_value()) {
    return GetFunctionInfoByScopeId(scope_id.value());
  }
  return nullptr;
}

std::optional<uint64_t> CaptureData::FindFunctionIdSlow(const FunctionInfo& function_info) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->FindFunctionIdSlow(function_info);
}

const FunctionInfo* CaptureData::GetFunctionInfoByScopeId(ScopeId scope_id) const {
  return scope_id_provider_->GetFunctionInfo(scope_id);
}

// InstrumentedFunction::function_virtual_address() was added in 1.82: if this is not available,
// we need to compute it from file_offset() to preserve compatibility with older captures.
// But note that ModuleData::ConvertFromOffsetInFileToVirtualAddress will use the ELF-specific
// computation of the virtual address as ModuleInfo::object_segments() was also added in 1.82:
// this is fine as that is the computation we were always using before 1.82.
void CaptureData::ComputeVirtualAddressOfInstrumentedFunctionsIfNecessary(
    const ModuleManager& module_manager) {
  uint64_t updated_function_count = 0;
  for (auto& [unused_function_id, instrumented_function] : instrumented_functions_) {
    if (instrumented_function.function_virtual_address() != 0) {
      continue;
    }

    const ModuleData* const module_data = module_manager.GetModuleByModuleIdentifier(
        ModuleIdentifier{instrumented_function.file_path(), instrumented_function.file_build_id()});
    if (module_data == nullptr) {
      continue;
    }

    const uint64_t virtual_address =
        module_data->ConvertFromOffsetInFileToVirtualAddress(instrumented_function.file_offset());
    instrumented_function.set_function_virtual_address(virtual_address);

    ORBIT_CHECK(scope_id_provider_);
    scope_id_provider_->UpdateFunctionInfoAddress(instrumented_function);

    ++updated_function_count;
  }

  if (updated_function_count > 0) {
    ORBIT_LOG("Set virtual address from offset for %u InstrumentedFunctions",
              updated_function_count);
  }
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
    const auto* function = GetFunctionInfoById(instrumented_function_id);
    if (function != nullptr) {
      ORBIT_LOG("Warning: Frame track for instrumented function \"%s\" is already enabled",
                function->pretty_name());
    }
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

std::optional<ScopeId> CaptureData::ProvideScopeId(
    const orbit_client_protos::TimerInfo& timer_info) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->ProvideId(timer_info);
}

[[nodiscard]] std::vector<ScopeId> CaptureData::GetAllProvidedScopeIds() const {
  return scope_id_provider_->GetAllProvidedScopeIds();
}

const ScopeInfo& CaptureData::GetScopeInfo(ScopeId scope_id) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->GetScopeInfo(scope_id);
}

std::optional<ScopeId> CaptureData::FunctionIdToScopeId(uint64_t function_id) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->FunctionIdToScopeId(function_id);
}

uint64_t CaptureData::ScopeIdToFunctionId(ScopeId scope_id) const {
  ORBIT_CHECK(scope_id_provider_);
  return scope_id_provider_->ScopeIdToFunctionId(scope_id);
}

const std::vector<uint64_t>* CaptureData::GetSortedTimerDurationsForScopeId(
    ScopeId scope_id) const {
  return all_scopes_->GetSortedTimerDurationsForScopeId(scope_id);
}

std::shared_ptr<const ScopeStatsCollection> CaptureData::GetAllScopeStatsCollection() const {
  return all_scopes_;
}

std::unique_ptr<const ScopeStatsCollection> CaptureData::CreateScopeStatsCollection(
    uint64_t min_tick, uint64_t max_tick) const {
  ORBIT_CHECK(scope_id_provider_);
  auto timers = GetAllScopeTimers(kAllValidScopeTypes, min_tick, max_tick);
  return std::make_unique<ScopeStatsCollection>(*scope_id_provider_, timers);
}

[[nodiscard]] std::vector<const TimerInfo*> CaptureData::GetAllScopeTimers(
    const absl::flat_hash_set<ScopeType> types, uint64_t min_tick, uint64_t max_tick) const {
  std::vector<const TimerInfo*> result;

  // The timers corresponding to dynamically instrumented functions and manual instrumentation
  // (kApiScope)  are stored in ThreadTracks. Hence, they're acquired separately from the manual
  // async (kApiScope).
  if (types.contains(ScopeType::kApiScope) ||
      types.contains(ScopeType::kDynamicallyInstrumentedFunction)) {
    for (const uint32_t thread_id : GetThreadTrackDataProvider()->GetAllThreadIds()) {
      const std::vector<const TimerInfo*> thread_track_timers =
          GetThreadTrackDataProvider()->GetTimers(thread_id, min_tick, max_tick);
      std::copy_if(std::begin(thread_track_timers), std::end(thread_track_timers),
                   std::back_inserter(result), [this, &types](const TimerInfo* timer) {
                     return types.contains(GetScopeInfo(ProvideScopeId(*timer).value()).GetType());
                   });
    }
  }

  if (types.contains(ScopeType::kApiScopeAsync)) {
    std::vector<const TimerInfo*> async_timer_infos = timer_data_manager_.GetTimers(
        orbit_client_protos::TimerInfo::kApiScopeAsync, min_tick, max_tick);

    result.insert(std::end(result), std::begin(async_timer_infos), std::end(async_timer_infos));
  }

  return result;
}

[[nodiscard]] std::vector<const TimerInfo*> CaptureData::GetTimersForScope(
    ScopeId scope_id, uint64_t min_tick, uint64_t max_tick) const {
  const std::vector<const TimerInfo*> all_timers =
      GetAllScopeTimers({GetScopeInfo(scope_id).GetType()}, min_tick, max_tick);
  std::vector<const TimerInfo*> result;
  std::copy_if(std::begin(all_timers), std::end(all_timers), std::back_inserter(result),
               [this, scope_id](const TimerInfo* timer) {
                 return scope_id_provider_->ProvideId(*timer) == scope_id;
               });
  return result;
}

[[nodiscard]] std::optional<ThreadStateSliceInfo>
CaptureData::FindThreadStateSliceInfoFromTimestamp(int64_t thread_id, uint64_t timestamp) const {
  absl::MutexLock lock{&thread_state_slices_mutex_};
  if (!thread_state_slices_.contains(thread_id)) {
    return std::nullopt;
  }

  const std::vector<ThreadStateSliceInfo>& thread_state_bar = thread_state_slices_.at(thread_id);
  auto slice = std::upper_bound(
      thread_state_bar.begin(), thread_state_bar.end(), timestamp,
      [](uint64_t a,
         const ThreadStateSliceInfo& b) -> bool {  // compare based on ending timestamps
        return a < b.end_timestamp_ns();
      });

  if (slice == thread_state_bar.end() || timestamp < slice->begin_timestamp_ns()) {
    return std::nullopt;
  }

  return *slice;
}

}  // namespace orbit_client_data
