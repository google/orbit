// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_EVENT_BUFFER_H_
#define ORBIT_CORE_EVENT_BUFFER_H_

#include <set>

#include "BlockChain.h"
#include "Callstack.h"
#include "Core.h"
#include "capture_data.pb.h"

#ifdef __linux
#include "LinuxUtils.h"
#endif

//-----------------------------------------------------------------------------
class EventBuffer {
 public:
  EventBuffer() : m_MaxTime(0), m_MinTime(LLONG_MAX) {}

  void Reset() {
    m_CallstackEvents.clear();
    m_MinTime = LLONG_MAX;
    m_MaxTime = 0;
  }
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::CallstackEvent> >&
  GetCallstacks() {
    return m_CallstackEvents;
  }
  Mutex& GetMutex() { return m_Mutex; }
  std::vector<orbit_client_protos::CallstackEvent> GetCallstackEvents(
      uint64_t a_TimeBegin, uint64_t a_TimeEnd, ThreadID a_ThreadId = 0);
  uint64_t GetMaxTime() const { return m_MaxTime; }
  uint64_t GetMinTime() const { return m_MinTime; }
  bool HasEvent() {
    ScopeLock lock(m_Mutex);
    return !m_CallstackEvents.empty();
  }

#ifdef __linux__
  size_t GetNumEvents() const;
#endif

  //-----------------------------------------------------------------------------
  void RegisterTime(uint64_t a_Time) {
    if (a_Time > m_MaxTime) m_MaxTime = a_Time;
    if (a_Time > 0 && a_Time < m_MinTime) m_MinTime = a_Time;
  }

  //-----------------------------------------------------------------------------
  void AddCallstackEvent(uint64_t time, CallstackID cs_hash,
                         ThreadID thread_id);

 private:
  Mutex m_Mutex;
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::CallstackEvent> >
      m_CallstackEvents;
  std::atomic<uint64_t> m_MaxTime;
  std::atomic<uint64_t> m_MinTime;
};

#endif  // ORBIT_CORE_EVENT_BUFFER_H_
