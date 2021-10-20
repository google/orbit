// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_TRACK_DATA_PROVIDER_H_
#define THREAD_TRACK_DATA_PROVIDER_H_

#include "ClientData/ThreadTrackDataManager.h"
#include "ClientData/TimerData.h"

namespace orbit_client_data {

struct TimerMetadata {
  bool is_empty;
  size_t number_of_timers;
  uint64_t min_time;
  uint64_t max_time;
  uint32_t depth;
  uint32_t process_id;
};

// Process timers and provide queries to get them using thread_id as a key.
class ThreadTrackDataProvider final {
 public:
  ThreadTrackDataProvider(bool is_data_from_saved_capture = false)
      : thread_track_data_manager_{
            std::make_unique<ThreadTrackDataManager>(is_data_from_saved_capture)} {};

  const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info) {
    return thread_track_data_manager_->AddTimer(std::move(timer_info));
  }

  const ScopeTreeTimerData* CreateScopeTreeTimerData(uint32_t thread_id) {
    return thread_track_data_manager_->CreateScopeTreeTimerData(thread_id);
  }

  // TODO(http://b/203515530): Replace this method by proper queries.
  [[nodiscard]] std::vector<const TimerChain*> GetAllThreadTimerChains() const;
  [[nodiscard]] std::vector<uint32_t> GetAllThreadIds() const;

  // For the following methods, we assume ScopeTreeTimerData is already been created for thread_id.
  std::vector<const TimerChain*> GetChains(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetChains();
  }

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint32_t thread_id, uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max()) const {
    return GetScopeTreeTimerData(thread_id)->GetTimers(min_tick, max_tick);
  }

  [[nodiscard]] const TimerMetadata GetTimerMetadata(uint32_t thread_id) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer) const;

  void OnCaptureComplete();

 private:
  [[nodiscard]] const ScopeTreeTimerData* GetScopeTreeTimerData(uint32_t thread_id) const {
    return thread_track_data_manager_->GetScopeTreeTimerData(thread_id);
  }
  std::unique_ptr<ThreadTrackDataManager> thread_track_data_manager_;
};

}  // namespace orbit_client_data

#endif  // THREAD_TRACK_DATA_PROVIDER_H_
