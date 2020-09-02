// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_EVENT_BUFFER_H_
#define ORBIT_CORE_EVENT_BUFFER_H_

#include <set>

#include "BlockChain.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "Threading.h"
#include "capture_data.pb.h"

class EventBuffer {
 public:
  EventBuffer() : m_MaxTime(0), m_MinTime(LLONG_MAX) {}

  void Reset() {
    callstack_events_.clear();
    m_MinTime = LLONG_MAX;
    m_MaxTime = 0;
  }
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::CallstackEvent> >& GetCallstacks() {
    return callstack_events_;
  }
  Mutex& GetMutex() { return m_Mutex; }
  std::vector<orbit_client_protos::CallstackEvent> GetCallstackEvents(
      uint64_t time_begin, uint64_t time_end,
      ThreadID thread_id = SamplingProfiler::kAllThreadsFakeTid);
  uint64_t GetMaxTime() const { return m_MaxTime; }
  uint64_t GetMinTime() const { return m_MinTime; }
  bool HasEvent() {
    ScopeLock lock(m_Mutex);
    return !callstack_events_.empty();
  }

#ifdef __linux__
  size_t GetNumEvents() const;
#endif

  void RegisterTime(uint64_t a_Time) {
    if (a_Time > m_MaxTime) m_MaxTime = a_Time;
    if (a_Time > 0 && a_Time < m_MinTime) m_MinTime = a_Time;
  }

  void AddCallstackEvent(uint64_t time, CallstackID cs_hash, ThreadID thread_id);

 private:
  Mutex m_Mutex;
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::CallstackEvent> > callstack_events_;
  std::atomic<uint64_t> m_MaxTime;
  std::atomic<uint64_t> m_MinTime;
};

#endif  // ORBIT_CORE_EVENT_BUFFER_H_
