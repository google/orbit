// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_TRACK_DATA_PROVIDER_H_
#define THREAD_TRACK_DATA_PROVIDER_H_

#include "ClientData/ThreadTrackDataManager.h"
#include "ClientData/TimerData.h"

namespace orbit_client_data {

// Process timers and provide queries to get them using thread_id as a key.
class ThreadTrackDataProvider final {
 public:
  ThreadTrackDataProvider(bool is_data_from_saved_capture = false)
      : thread_track_data_manager_{
            std::make_unique<ThreadTrackDataManager>(is_data_from_saved_capture)} {};

  const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info);
  void CreateScopeTreeTimerData(uint32_t thread_id) const {
    thread_track_data_manager_->GetOrCreateScopeTreeTimerData(thread_id);
  }

  // TODO(http://b/???): This function is used to jump to specific timers. Should be replaced by
  // proper queries later.
  std::vector<const TimerChain*> GetAllThreadTimerChains() const;
  [[nodiscard]] std::vector<uint32_t> GetAllThreadId() const;

  // For all this queries, we assume the ScopeTreeTimerData already exists.
  std::vector<const TimerChain*> GetChains(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetChains();
  }

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimersAtDepth(
      uint32_t thread_id, uint32_t depth, uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max(), uint64_t resolution = 1000) const {
    return GetScopeTreeTimerData(thread_id)->GetTimersAtDepth(depth, min_tick, max_tick,
                                                              resolution);
  }

  [[nodiscard]] bool IsEmpty(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->IsEmpty();
  }

  [[nodiscard]] size_t GetNumberOfTimers(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetNumberOfTimers();
  }

  [[nodiscard]] uint64_t GetMinTime(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetMinTime();
  }

  [[nodiscard]] uint64_t GetMaxTime(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetMaxTime();
  }

  [[nodiscard]] uint32_t GetDepth(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetDepth();
  }

  [[nodiscard]] uint32_t GetProcessId(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetProcessId();
  }

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
  [[nodiscard]] ScopeTreeTimerData* GetOrCreateScopeTreeTimerData(uint32_t thread_id) const {
    return thread_track_data_manager_->GetOrCreateScopeTreeTimerData(thread_id);
  }
  [[nodiscard]] ScopeTreeTimerData* GetScopeTreeTimerData(uint32_t thread_id) const {
    return thread_track_data_manager_->GetScopeTreeTimerData(thread_id);
  }

  std::unique_ptr<ThreadTrackDataManager> thread_track_data_manager_;
};

}  // namespace orbit_client_data

#endif  // THREAD_TRACK_DATA_PROVIDER_H_
