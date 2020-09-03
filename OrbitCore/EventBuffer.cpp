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
    uint64_t time_begin, uint64_t time_end, int32_t thread_id /*= kAllThreadsFakeTid*/) const {
  std::vector<CallstackEvent> callstack_events;
  for (auto& pair : callstack_events_) {
    const int32_t callstack_thread_id = pair.first;
    const std::map<uint64_t, CallstackEvent>& callstacks = pair.second;

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

bool EventBuffer::HasEvent() {
  ScopeLock lock(mutex_);
  if (callstack_events_.empty()) {
    return false;
  }
  for (const auto& pair : callstack_events_) {
    if (!pair.second.empty()) {
      return true;
    }
  }
  return false;
}

void EventBuffer::AddCallstackEvent(uint64_t time, CallstackID cs_hash, int32_t thread_id) {
  ScopeLock lock(mutex_);
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

size_t EventBuffer::GetNumEvents() const {
  size_t num_events = 0;
  for (auto& pair : callstack_events_) {
    num_events += pair.second.size();
  }

  return num_events;
}
