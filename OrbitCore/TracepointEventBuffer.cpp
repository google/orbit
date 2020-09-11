// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointEventBuffer.h"

#include "SamplingProfiler.h"

using orbit_client_protos::TracepointEventInfo;

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

void TracepointEventBuffer::AddTracepointEvent(uint64_t time, uint64_t tracepoint_hash,
                                               ThreadID thread_id) {
  ScopeLock lock(mutex_);
  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map =
      tracepoint_events_[thread_id];
  orbit_client_protos::TracepointEventInfo event;
  event.set_time(time);
  event.set_tracepoint_info_key(tracepoint_hash);
  event.set_tid(thread_id);
  event_map[time] = event;

  // Add all tracepoint events to "all threads".
  std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& event_map_all_threads =
      tracepoint_events_[SamplingProfiler::kAllThreadsFakeTid];
  orbit_client_protos::TracepointEventInfo event_all_threads;
  event_all_threads.set_time(time);
  event_all_threads.set_tracepoint_info_key(tracepoint_hash);
  event_all_threads.set_tid(SamplingProfiler::kAllThreadsFakeTid);
  event_map_all_threads[time] = event_all_threads;

  RegisterTime(time);
}

size_t TracepointEventBuffer::GetNumEvents() const {
  ScopeLock lock(mutex_);
  size_t num_events = 0;
  for (const auto& pair : tracepoint_events_) {
    num_events += pair.second.size();
  }

  return num_events;
}

bool TracepointEventBuffer::HasEvent() const {
  ScopeLock lock(mutex_);
  if (tracepoint_events_.empty()) {
    return false;
  }
  for (const auto& pair : tracepoint_events_) {
    if (!pair.second.empty()) {
      return true;
    }
  }
  return false;
}

void TracepointEventBuffer::Reset() {
  ScopeLock lock(mutex_);
  tracepoint_events_.clear();
  min_time_ = std::numeric_limits<uint64_t>::max();
  max_time_ = 0;
}

uint64_t TracepointEventBuffer::max_time() const { return max_time_; }
uint64_t TracepointEventBuffer::min_time() const { return min_time_; }

std::map<ThreadID, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >
TracepointEventBuffer::tracepoint_events() {
  return tracepoint_events_;
}

void TracepointEventBuffer::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

Mutex& TracepointEventBuffer::mutex() { return mutex_; }