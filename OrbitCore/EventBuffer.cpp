// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EventBuffer.h"

#include "EventTracer.h"
#include "Params.h"
#include "SamplingProfiler.h"

using orbit_client_protos::CallstackEvent;

EventTracer GEventTracer;

std::vector<CallstackEvent> EventBuffer::GetCallstackEvents(
    uint64_t time_begin, uint64_t time_end, ThreadID thread_id /*= kAllThreadsFakeTid*/) {
  std::vector<CallstackEvent> callstack_events;
  for (auto& pair : callstack_events_) {
    const ThreadID callstack_thread_id = pair.first;
    std::map<uint64_t, CallstackEvent>& callstacks = pair.second;

    if (thread_id == SamplingProfiler::kAllThreadsFakeTid || callstack_thread_id == thread_id) {
      for (auto it = callstacks.lower_bound(time_begin); it != callstacks.end(); ++it) {
        uint64_t time = it->first;
        if (time < time_end) {
          callstack_events.push_back(it->second);
        }
      }
    }
  }

  return callstack_events;
}

void EventBuffer::AddCallstackEvent(uint64_t time, CallstackID cs_hash, ThreadID thread_id) {
  ScopeLock lock(m_Mutex);
  std::map<uint64_t, CallstackEvent>& event_map = callstack_events_[thread_id];
  CallstackEvent event;
  event.set_time(time);
  event.set_callstack_hash(cs_hash);
  event.set_thread_id(thread_id);
  event_map[time] = event;

  // Add all callstack events to "all threads".
  std::map<uint64_t, CallstackEvent>& event_map_all_threads =
      callstack_events_[SamplingProfiler::kAllThreadsFakeTid];
  CallstackEvent event_all_threads;
  event_all_threads.set_time(time);
  event_all_threads.set_callstack_hash(cs_hash);
  event_all_threads.set_thread_id(SamplingProfiler::kAllThreadsFakeTid);
  event_map_all_threads[time] = event_all_threads;

  RegisterTime(time);
}

#ifdef __linux
size_t EventBuffer::GetNumEvents() const {
  size_t numEvents = 0;
  for (auto& pair : callstack_events_) {
    numEvents += pair.second.size();
  }

  return numEvents;
}
#endif
