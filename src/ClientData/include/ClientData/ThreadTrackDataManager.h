// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_TRACK_DATA_MANAGER_H_
#define THREAD_TRACK_DATA_MANAGER_H_

#include <absl/synchronization/mutex.h>

#include <vector>

#include "ClientData/TimerData.h"
#include "OrbitBase/Append.h"
#include "ScopeTreeTimerData.h"
#include "TimerDataManager.h"

namespace orbit_client_data {

// Creates and stores data from Thread Tracks in a thread-safe way, using thread_id as the key.
class ThreadTrackDataManager final {
 public:
  ThreadTrackDataManager(bool is_data_from_saved_capture = false)
      : scope_tree_update_type_(is_data_from_saved_capture
                                    ? ScopeTreeTimerData::ScopeTreeUpdateType::kOnCaptureComplete
                                    : ScopeTreeTimerData::ScopeTreeUpdateType::kAlways){};

  ScopeTreeTimerData* CreateScopeTreeTimerData(uint32_t thread_id) {
    absl::MutexLock lock(&mutex_);
    scope_tree_timer_data_map_.insert_or_assign(
        thread_id, std::make_unique<ScopeTreeTimerData>(thread_id, scope_tree_update_type_));
    return scope_tree_timer_data_map_.at(thread_id).get();
  };

  std::vector<const TimerChain*> GetAllThreadTimerChains() const {
    absl::MutexLock lock(&mutex_);
    std::vector<const TimerChain*> chains;
    for (const auto& [unused_tid, scope_tree_timer_data] : scope_tree_timer_data_map_) {
      orbit_base::Append(chains, scope_tree_timer_data->GetTimerData()->GetChains());
    }
    return chains;
  }

  void OnCaptureComplete() {
    absl::MutexLock lock(&mutex_);
    // Build ScopeTree from timer chains when we are loading a capture.
    if (scope_tree_update_type_ == ScopeTreeTimerData::ScopeTreeUpdateType::kOnCaptureComplete) {
      for (const auto& [unused_tid, scope_tree_timer_data] : scope_tree_timer_data_map_) {
        scope_tree_timer_data->BuildScopeTreeFromTimerData();
      }
    }
  }

 private:
  mutable absl::Mutex mutex_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<ScopeTreeTimerData>> scope_tree_timer_data_map_
      GUARDED_BY(mutex_);
  ScopeTreeTimerData::ScopeTreeUpdateType scope_tree_update_type_;
};

}  // namespace orbit_client_data

#endif  // THREAD_TRACK_DATA_MANAGER_H_
