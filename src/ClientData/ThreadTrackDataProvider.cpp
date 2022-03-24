// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ThreadTrackDataProvider.h"

#include <cstdint>

#include "ClientData/ScopeIdConstants.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

std::vector<uint32_t> ThreadTrackDataProvider::GetAllThreadIds() const {
  std::vector<uint32_t> all_thread_id;
  for (const ScopeTreeTimerData* scope_tree_timer_data :
       thread_track_data_manager_->GetAllScopeTreeTimerData()) {
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

const TimerInfo* ThreadTrackDataProvider::GetLeft(const TimerInfo& timer) const {
  return GetScopeTreeTimerData(timer.thread_id())->GetLeft(timer);
}

const TimerInfo* ThreadTrackDataProvider::GetRight(const TimerInfo& timer) const {
  return GetScopeTreeTimerData(timer.thread_id())->GetRight(timer);
}

const TimerInfo* ThreadTrackDataProvider::GetUp(const TimerInfo& timer) const {
  return GetScopeTreeTimerData(timer.thread_id())->GetUp(timer);
}

const TimerInfo* ThreadTrackDataProvider::GetDown(const TimerInfo& timer) const {
  return GetScopeTreeTimerData(timer.thread_id())->GetDown(timer);
}

void ThreadTrackDataProvider::OnCaptureComplete() {
  // Update data if needed after capture is completed.
  for (ScopeTreeTimerData* scope_tree_timer_data :
       thread_track_data_manager_->GetAllScopeTreeTimerData()) {
    scope_tree_timer_data->OnCaptureComplete();
  }
  UpdateTimerDurations();
}

const std::vector<uint64_t>* ThreadTrackDataProvider::GetSortedTimerDurationsForScopeId(
    uint64_t scope_id) const {
  const auto it = timer_durations_.find(scope_id);
  if (it == timer_durations_.end()) return nullptr;
  return &it->second;
}

void ThreadTrackDataProvider::UpdateTimerDurations() {
  ORBIT_SCOPE_FUNCTION;
  timer_durations_.clear();
  ORBIT_CHECK(scope_id_provider_ != nullptr);

  const std::vector<const orbit_client_data::TimerChain*> chains = GetAllThreadTimerChains();

  for (const orbit_client_data::TimerChain* chain : chains) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer = block[i];
        const uint64_t scope_id = scope_id_provider_->ProvideId(timer);

        if (scope_id == orbit_client_data::kInvalidScopeId) continue;

        timer_durations_[scope_id].push_back(timer.end() - timer.start());
      }
    }
  }

  for (auto& [id, timer_durations] : timer_durations_) {
    std::sort(timer_durations.begin(), timer_durations.end());
  }
}

}  // namespace orbit_client_data