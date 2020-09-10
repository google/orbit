// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
#define ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_

#include <set>

#include "BlockChain.h"
#include "SamplingProfiler.h"
#include "Threading.h"
#include "capture_data.pb.h"

class TracepointEventBuffer {
 public:
  TracepointEventBuffer() : m_MaxTime(0), m_MinTime(LLONG_MAX) {}

  void Reset() {
    tracepoint_events_.clear();
    m_MinTime = LLONG_MAX;
    m_MaxTime = 0;
  }
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >&
  GetTracepoints() {
    return tracepoint_events_;
  }
  Mutex& GetMutex() { return m_Mutex; }
  std::vector<orbit_client_protos::TracepointEventInfo> GetTracepointEvents(
      uint64_t time_begin, uint64_t time_end,
      ThreadID thread_id = SamplingProfiler::kAllThreadsFakeTid);
  uint64_t GetMaxTime() const { return m_MaxTime; }
  uint64_t GetMinTime() const { return m_MinTime; }
  bool HasEvent() {
    ScopeLock lock(m_Mutex);
    return !tracepoint_events_.empty();
  }

#ifdef __linux__
  size_t GetNumEvents() const;
#endif

  void RegisterTime(uint64_t a_Time) {
    if (a_Time > m_MaxTime) m_MaxTime = a_Time;
    if (a_Time > 0 && a_Time < m_MinTime) m_MinTime = a_Time;
  }

  void AddTracepointEvent(uint64_t time, uint64_t hash, ThreadID thread_id);

 private:
  Mutex m_Mutex;
  std::atomic<uint64_t> m_MaxTime;
  std::atomic<uint64_t> m_MinTime;
  std::map<ThreadID, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >
      tracepoint_events_;
};

#endif  // ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
