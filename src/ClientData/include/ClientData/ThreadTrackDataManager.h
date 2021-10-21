// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_TRACK_DATA_MANAGER_H_
#define THREAD_TRACK_DATA_MANAGER_H_

#include <absl/container/flat_hash_map.h>
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

  const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info) {
    absl::MutexLock lock(&mutex_);
    uint32_t thread_id = timer_info.thread_id();
    // Get or create ScopeTreeTimerData optimized to only make one query to the map, as AddTimer
    // will be executed many times.
    auto [it, inserted] = scope_tree_timer_data_map_.try_emplace(thread_id, nullptr);
    if (inserted) {
      it->second = std::make_unique<ScopeTreeTimerData>(thread_id, scope_tree_update_type_);
    }
    return it->second->AddTimer(std::move(timer_info));
  }

  const ScopeTreeTimerData* GetScopeTreeTimerData(uint32_t thread_id) const {
    absl::MutexLock lock(&mutex_);
    auto it = scope_tree_timer_data_map_.find(thread_id);
    if (it == scope_tree_timer_data_map_.end()) {
      return nullptr;
    }
    return it->second.get();
  };

  // This function should be used only for Tracks that needs to be there before a Timer appears.
  const ScopeTreeTimerData* CreateScopeTreeTimerData(uint32_t thread_id) {
    absl::MutexLock lock(&mutex_);
    auto [it, unused_inserted] = scope_tree_timer_data_map_.try_emplace(
        thread_id, std::make_unique<ScopeTreeTimerData>(thread_id, scope_tree_update_type_));
    return it->second.get();
  };

  [[nodiscard]] std::vector<ScopeTreeTimerData*> GetAllScopeTreeTimerData() const {
    absl::MutexLock lock(&mutex_);
    std::vector<ScopeTreeTimerData*> all_scope_tree_timer_data;
    all_scope_tree_timer_data.reserve(scope_tree_timer_data_map_.size());
    for (const auto& [unused_tid, scope_tree_timer_data] : scope_tree_timer_data_map_) {
      all_scope_tree_timer_data.push_back(scope_tree_timer_data.get());
    }
    return all_scope_tree_timer_data;
  }

 private:
  mutable absl::Mutex mutex_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<ScopeTreeTimerData>> scope_tree_timer_data_map_
      ABSL_GUARDED_BY(mutex_);
  ScopeTreeTimerData::ScopeTreeUpdateType scope_tree_update_type_;
};

}  // namespace orbit_client_data

#endif  // THREAD_TRACK_DATA_MANAGER_H_
