// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/TracepointEventBuffer.h"

using orbit_client_protos::TracepointEventInfo;

const int32_t TracepointEventBuffer::kNotTargetProcessThreadId = -2;
const int32_t TracepointEventBuffer::kAllTracepointsFakeTid = -3;

void TracepointEventBuffer::AddTracepointEventAndMapToThreads(uint64_t time,
                                                              uint64_t tracepoint_hash,
                                                              int32_t process_id, int32_t thread_id,
                                                              int32_t cpu,
                                                              bool is_same_pid_as_target) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  num_total_tracepoints_++;
  if (!is_same_pid_as_target) {
    std::map<uint64_t, orbit_client_protos::TracepointEventInfo>&
        event_map_tracepoints_not_in_target_process = tracepoint_events_[kNotTargetProcessThreadId];
    orbit_client_protos::TracepointEventInfo event;
    event.set_time(time);
    event.set_tracepoint_info_key(tracepoint_hash);
    event.set_tid(thread_id);
    event.set_pid(process_id);
    event.set_cpu(cpu);
    event_map_tracepoints_not_in_target_process[time] = std::move(event);
    return;
  }

  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map =
      tracepoint_events_[thread_id];
  orbit_client_protos::TracepointEventInfo event;
  event.set_time(time);
  event.set_tracepoint_info_key(tracepoint_hash);
  event.set_tid(thread_id);
  event.set_pid(process_id);
  event.set_cpu(cpu);
  event_map[time] = std::move(event);
}

[[nodiscard]] const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>&
TracepointEventBuffer::GetTracepointsOfThread(int32_t thread_id) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  static std::map<uint64_t, orbit_client_protos::TracepointEventInfo> empty;
  const auto& it = tracepoint_events_.find(thread_id);
  if (it == tracepoint_events_.end()) {
    return empty;
  }
  return it->second;
}

static void ForEachTracepointEventInEventMapInTimeRange(
    uint64_t min_tick, uint64_t max_tick,
    const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& time_to_tracepoint_events,
    const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) {
  for (auto time_to_tracepoint_event = time_to_tracepoint_events.lower_bound(min_tick);
       time_to_tracepoint_event != time_to_tracepoint_events.upper_bound(max_tick);
       ++time_to_tracepoint_event) {
    action(time_to_tracepoint_event->second);
  }
}

void TracepointEventBuffer::ForEachTracepointEventOfThreadInTimeRange(
    int32_t thread_id, uint64_t min_tick, uint64_t max_tick,
    const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (thread_id == TracepointEventBuffer::kAllTracepointsFakeTid) {
    for (const auto& entry : tracepoint_events_) {
      ForEachTracepointEventInEventMapInTimeRange(min_tick, max_tick, entry.second, action);
    }
  } else if (thread_id == -1 /*TODO(b/166238019) Use a proper constant for "all fake thread id"*/) {
    for (const auto& entry : tracepoint_events_) {
      if (entry.first != kNotTargetProcessThreadId) {
        ForEachTracepointEventInEventMapInTimeRange(min_tick, max_tick, entry.second, action);
      }
    }
  } else {
    ForEachTracepointEventInEventMapInTimeRange(min_tick, max_tick,
                                                GetTracepointsOfThread(thread_id), action);
  }
}

uint32_t TracepointEventBuffer::GetNumTracepointsForThreadId(int32_t thread_id) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (thread_id == TracepointEventBuffer::kAllTracepointsFakeTid) {
    return num_total_tracepoints_;
  }
  if (thread_id == -1 /*TODO(b/166238019) Use a proper constant for "all fake thread id"*/) {
    const auto not_target_process_tracepoints_it =
        tracepoint_events_.find(kNotTargetProcessThreadId);
    if (not_target_process_tracepoints_it == tracepoint_events_.end()) {
      return num_total_tracepoints_;
    }
    return num_total_tracepoints_ - not_target_process_tracepoints_it->second.size();
  }

  return GetTracepointsOfThread(thread_id).size();
}
