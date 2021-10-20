// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ThreadTrackDataProvider.h"

namespace orbit_client_data {

const orbit_client_protos::TimerInfo& ThreadTrackDataProvider::AddTimer(
    orbit_client_protos::TimerInfo timer_info) {
  uint32_t thread_id = timer_info.thread_id();
  auto scope_tree_timer_data = GetOrCreateScopeTreeTimerData(thread_id);
  return scope_tree_timer_data->AddTimer(timer_info);
}

std::vector<uint32_t> ThreadTrackDataProvider::GetAllThreadIds() const {
  std::vector<uint32_t> all_thread_id;
  for (const auto scope_tree_timer_data : thread_track_data_manager_->GetAllScopeTreeTimerData()) {
    all_thread_id.push_back(scope_tree_timer_data->GetThreadId());
  }
  return all_thread_id;
}

std::vector<const TimerChain*> ThreadTrackDataProvider::GetAllThreadTimerChains() const {
  std::vector<const TimerChain*> chains;
  for (const uint32_t thread_id : GetAllThreadIds()) {
    orbit_base::Append(chains, GetChains(thread_id));
  }
  return chains;
}

[[nodiscard]] const TimerMetadata ThreadTrackDataProvider::GetTimerMetadata(
    uint32_t thread_id) const {
  ScopeTreeTimerData* scope_tree_timer_data = GetOrCreateScopeTreeTimerData(thread_id);
  CHECK(scope_tree_timer_data != nullptr);
  TimerMetadata timer_metadata;
  timer_metadata.is_empty = scope_tree_timer_data->IsEmpty();
  timer_metadata.number_of_timers = scope_tree_timer_data->GetNumberOfTimers();
  timer_metadata.min_time = scope_tree_timer_data->GetMinTime();
  timer_metadata.max_time = scope_tree_timer_data->GetMaxTime();
  timer_metadata.depth = scope_tree_timer_data->GetDepth();
  timer_metadata.process_id = scope_tree_timer_data->GetProcessId();
  return timer_metadata;
}

const orbit_client_protos::TimerInfo* ThreadTrackDataProvider::GetLeft(
    const orbit_client_protos::TimerInfo& timer) const {
  uint32_t thread_id = timer.thread_id();
  return GetScopeTreeTimerData(thread_id)->GetLeft(timer);
}

const orbit_client_protos::TimerInfo* ThreadTrackDataProvider::GetRight(
    const orbit_client_protos::TimerInfo& timer) const {
  uint32_t thread_id = timer.thread_id();
  return GetScopeTreeTimerData(thread_id)->GetRight(timer);
}

const orbit_client_protos::TimerInfo* ThreadTrackDataProvider::GetUp(
    const orbit_client_protos::TimerInfo& timer) const {
  uint32_t thread_id = timer.thread_id();
  return GetScopeTreeTimerData(thread_id)->GetUp(timer);
}

const orbit_client_protos::TimerInfo* ThreadTrackDataProvider::GetDown(
    const orbit_client_protos::TimerInfo& timer) const {
  uint32_t thread_id = timer.thread_id();
  return GetScopeTreeTimerData(thread_id)->GetDown(timer);
}

void ThreadTrackDataProvider::OnCaptureComplete() {
  // Update data if needed after capture is completed.
  for (const auto scope_tree_timer_data : thread_track_data_manager_->GetAllScopeTreeTimerData()) {
    scope_tree_timer_data->OnCaptureComplete();
  }
}

}  // namespace orbit_client_data