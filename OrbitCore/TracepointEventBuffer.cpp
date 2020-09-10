// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointEventBuffer.h"

#include "SamplingProfiler.h"
#include "TracepointEventTracer.h"

using orbit_client_protos::TracepointEventInfo;

TracepointEventTracer GTracepointEventTracer;

std::vector<TracepointEventInfo> TracepointEventBuffer::GetTracepointEvents(
    uint64_t time_begin, uint64_t time_end, ThreadID thread_id /*= kAllThreadsFakeTid*/) {
  std::vector<TracepointEventInfo> tracepoint_events;
  for (auto& pair : tracepoint_events_) {
    const ThreadID tracepoint_thread_id = pair.first;
    std::map<uint64_t, TracepointEventInfo>& tracepoints = pair.second;

    CHECK(thread_id == SamplingProfiler::kAllThreadsFakeTid || tracepoint_thread_id == thread_id);
    for (auto it = tracepoints.lower_bound(time_begin); it != tracepoints.end(); ++it) {
      uint64_t time = it->first;
      if (time < time_end) {
        tracepoint_events.push_back(it->second);
      }
    }
  }

  return tracepoint_events;
}

void TracepointEventBuffer::AddTracepointEvent(uint64_t time, uint64_t hash, ThreadID thread_id) {
  ScopeLock lock(m_Mutex);
  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map =
      tracepoint_events_[thread_id];
  orbit_client_protos::TracepointEventInfo event;
  event.set_time(time);
  event.set_tracepoint_info_key(hash);
  event.set_tid(thread_id);
  event_map[time] = event;

  // Add all tracepoint events to "all threads".
  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map_all_threads =
      tracepoint_events_[SamplingProfiler::kAllThreadsFakeTid];
  orbit_client_protos::TracepointEventInfo event_all_threads;
  event_all_threads.set_time(time);
  event_all_threads.set_tracepoint_info_key(hash);
  event_all_threads.set_tid(SamplingProfiler::kAllThreadsFakeTid);
  event_map_all_threads[time] = event_all_threads;

  RegisterTime(time);
}

#ifdef __linux
size_t TracepointEventBuffer::GetNumEvents() const {
  size_t numEvents = 0;
  for (auto& pair : tracepoint_events_) {
    numEvents += pair.second.size();
  }

  return numEvents;
}
#endif