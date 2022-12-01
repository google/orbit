// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ThreadTrackDataProvider.h"

#include "OrbitBase/Append.h"

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
}

}  // namespace orbit_client_data