// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointEventBuffer.h"

using orbit_client_protos::TracepointEventInfo;

void TracepointEventBuffer::AddTracepointEventAndMapToThreads(uint64_t time,
                                                              uint64_t tracepoint_hash,
                                                              int32_t process_id, int32_t thread_id,
                                                              int32_t cpu,
                                                              bool is_same_pid_as_target) {
  if (!is_same_pid_as_target) {
    return;
  }

  /*TODO: tracepoint events with !is_same_pid_as_target will also have to be
   * stored when implementing the track showing tracepoint events from all processes in the system*/

  ScopeLock lock(mutex_);
  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map =
      tracepoint_events_[thread_id];
  orbit_client_protos::TracepointEventInfo event;
  event.set_time(time);
  event.set_tracepoint_info_key(tracepoint_hash);
  event.set_tid(thread_id);
  event.set_pid(process_id);
  event.set_cpu(cpu);
  event_map[time] = std::move(event);

  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map_all_threads =
      tracepoint_events_[SamplingProfiler::kAllThreadsFakeTid];
  orbit_client_protos::TracepointEventInfo event_all_threads;
  event_all_threads.set_time(time);
  event_all_threads.set_tracepoint_info_key(tracepoint_hash);
  event_all_threads.set_tid(thread_id);
  event_all_threads.set_pid(process_id);
  event_all_threads.set_cpu(cpu);
  event_map_all_threads[time] = std::move(event_all_threads);

  RegisterTime(time);
}

[[nodiscard]] const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>&
TracepointEventBuffer::GetTracepointsOfThread(int32_t thread_id) const {
  static std::map<uint64_t, orbit_client_protos::TracepointEventInfo> empty;
  const auto& it = tracepoint_events_.find(thread_id);
  if (it == tracepoint_events_.end()) {
    return empty;
  }
  return it->second;
}

void TracepointEventBuffer::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

Mutex& TracepointEventBuffer::mutex() { return mutex_; }