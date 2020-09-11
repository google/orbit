// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
#define ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_

#include <limits>
#include <set>

#include "SamplingProfiler.h"
#include "capture_data.pb.h"

class TracepointEventBuffer {
 public:
  TracepointEventBuffer() : max_time_(0), min_time_(std::numeric_limits<uint64_t>::max()) {}

  std::vector<orbit_client_protos::TracepointEventInfo> GetTracepointEventsInTimeRangeForThreadId(
      uint64_t time_begin, uint64_t time_end,
      int32_t thread_id = SamplingProfiler::kAllThreadsFakeTid);

  void AddTracepointEventAndMapToThreads(uint64_t time, uint64_t tracepoint_hash,
                                         int32_t thread_id);

  std::map<int32_t, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >
  tracepoint_events();

  Mutex& mutex();

  uint64_t max_time() const;
  uint64_t min_time() const;

  bool HasEvent() const;

 private:
  void RegisterTime(uint64_t time);

  mutable Mutex mutex_;
  std::atomic<uint64_t> max_time_;
  std::atomic<uint64_t> min_time_;
  std::map<int32_t, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >
      tracepoint_events_;
};

#endif  // ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
