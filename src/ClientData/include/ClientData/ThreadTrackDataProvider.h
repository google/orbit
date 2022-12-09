// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_TRACK_DATA_PROVIDER_H_
#define THREAD_TRACK_DATA_PROVIDER_H_

#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeTreeTimerData.h"
#include "ClientData/ThreadTrackDataManager.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerData.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

// Using thread_id as a key, process timers and provide queries to get all in a range (start,end) as
// well as metadata about them.
class ThreadTrackDataProvider final {
 public:
  explicit ThreadTrackDataProvider(bool is_data_from_saved_capture = false)
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
  [[nodiscard]] std::vector<const TimerChain*> GetChains(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetChains();
  }

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint32_t thread_id, uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max()) const {
    return GetScopeTreeTimerData(thread_id)->GetTimers(min_tick, max_tick);
  }

  // This method avoids returning two timers that map to the same pixel, so is especially useful
  // when many timers map to the same pixel (zooming-out for example). The overall complexity is
  // O(log(num_timers) * resolution). Resolution should be the pixel width of the area where timers
  // will be drawn.
  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimersAtDepthDiscretized(
      uint32_t thread_id, uint32_t depth, uint32_t resolution, uint64_t start_ns,
      uint64_t end_ns) const {
    return GetScopeTreeTimerData(thread_id)->GetTimersAtDepthDiscretized(depth, resolution,
                                                                         start_ns, end_ns);
  }

  // Metadata queries
  [[nodiscard]] bool IsEmpty(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->IsEmpty();
  };
  [[nodiscard]] size_t GetNumberOfTimers(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetNumberOfTimers();
  };
  [[nodiscard]] uint64_t GetMinTime(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetMinTime();
  };
  [[nodiscard]] uint64_t GetMaxTime(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetMaxTime();
  };
  [[nodiscard]] uint32_t GetDepth(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetDepth();
  };
  [[nodiscard]] uint32_t GetProcessId(uint32_t thread_id) const {
    return GetScopeTreeTimerData(thread_id)->GetProcessId();
  };

  // Relative Timers query
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
