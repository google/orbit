// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EventBuffer.h"

#include "Capture.h"
#include "EventTracer.h"
#include "Params.h"
#include "SamplingProfiler.h"
#include "Serialization.h"

EventTracer GEventTracer;

//-----------------------------------------------------------------------------
std::vector<CallstackEvent> EventBuffer::GetCallstackEvents(
    uint64_t a_TimeBegin, uint64_t a_TimeEnd, ThreadID a_ThreadId /*= 0*/) {
  std::vector<CallstackEvent> callstackEvents;
  for (auto& pair : data_->callstack_events()) {
    ThreadID threadID = pair.first;
    const Uint64ToCallstackEvent& callstacks = pair.second;

    if (a_ThreadId == 0 || threadID == a_ThreadId) {
      for (auto it = callstacks.data().begin(); it != callstacks.data().end();
           ++it) {
        uint64_t time = it->first;
        if (a_TimeBegin <= time && time < a_TimeEnd) {
          callstackEvents.push_back(it->second);
        }
      }
    }
  }

  return callstackEvents;
}

//-----------------------------------------------------------------------------
void EventBuffer::AddCallstackEvent(uint64_t time, CallstackID cs_hash,
                                    ThreadID thread_id) {
  ScopeLock lock(m_Mutex);

  CallstackEvent event;
  event.set_time(time);
  event.set_callstack_id(cs_hash);
  event.set_thread_id(thread_id);
  Uint64ToCallstackEvent& event_map =
      (*data_->mutable_callstack_events())[thread_id];
  (*event_map.mutable_data())[time] = event;

  // Add all callstack events to "thread 0".
  CallstackEvent event0;
  event0.set_time(time);
  event0.set_callstack_id(cs_hash);
  event0.set_thread_id(0);
  Uint64ToCallstackEvent& event_map_0 = (*data_->mutable_callstack_events())[0];
  (*event_map_0.mutable_data())[time] = event0;

  RegisterTime(time);
}

#ifdef __linux
//-----------------------------------------------------------------------------
size_t EventBuffer::GetNumEvents() const {
  size_t numEvents = 0;
  for (auto& pair : data_->callstack_events()) {
    numEvents += pair.second.data_size();
  }

  return numEvents;
}
#endif
