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
  for (auto& pair : m_CallstackEvents) {
    ThreadID threadID = pair.first;
    std::map<uint64_t, CallstackEvent>& callstacks = pair.second;

    if (a_ThreadId == 0 || threadID == a_ThreadId) {
      for (auto it = callstacks.lower_bound(a_TimeBegin);
           it != callstacks.end(); ++it) {
        uint64_t time = it->first;
        if (time < a_TimeEnd) {
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
  std::map<uint64_t, CallstackEvent>& event_map = m_CallstackEvents[thread_id];
  event_map[time] = CallstackEvent(time, cs_hash, thread_id);

  // Add all callstack events to "thread 0".
  std::map<uint64_t, CallstackEvent>& event_map_0 = m_CallstackEvents[0];
  event_map_0[time] = CallstackEvent(time, cs_hash, 0);

  RegisterTime(time);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CallstackEvent, 1) {
  ORBIT_NVP_VAL(1, m_Time);
  ORBIT_NVP_VAL(0, m_Id);
  ORBIT_NVP_VAL(0, m_TID);
}

#ifdef __linux
//-----------------------------------------------------------------------------
size_t EventBuffer::GetNumEvents() const {
  size_t numEvents = 0;
  for (auto& pair : m_CallstackEvents) {
    numEvents += pair.second.size();
  }

  return numEvents;
}
#endif
