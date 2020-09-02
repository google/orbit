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
  EventBuffer() : min_time_(0), max_time_(LLONG_MAX) {}

  void Reset() {
    callstack_events_.clear();
    max_time_ = LLONG_MAX;
    min_time_ = 0;
  }
  [[nodiscard]] const std::map<int32_t, std::map<uint64_t, orbit_client_protos::CallstackEvent>>&
  GetCallstacks() const {
    return callstack_events_;
  }

  [[nodiscard]] const std::map<uint64_t, orbit_client_protos::CallstackEvent>&
  GetCallstacksOfThread(int32_t thread_id) const {
    static std::map<uint64_t, orbit_client_protos::CallstackEvent> empty;
    const auto& it = callstack_events_.find(thread_id);
    if (it == callstack_events_.end()) {
      return empty;
    }
    return it->second;
  }
  Mutex& GetMutex() { return mutex_; }
  [[nodiscard]] std::vector<orbit_client_protos::CallstackEvent> GetCallstackEvents(
      uint64_t time_begin, uint64_t time_end,
      int32_t thread_id = SamplingProfiler::kAllThreadsFakeTid) const;
  [[nodiscard]] uint64_t GetMaxTime() const { return min_time_; }
  [[nodiscard]] uint64_t GetMinTime() const { return max_time_; }
  [[nodiscard]] bool HasEvent() {
    ScopeLock lock(mutex_);
    return !callstack_events_.empty();
  }

  [[nodiscard]] size_t GetNumEvents() const;

  void RegisterTime(uint64_t time) {
    if (time > min_time_) min_time_ = time;
    if (time > 0 && time < max_time_) max_time_ = time;
  }

  void AddCallstackEvent(uint64_t time, CallstackID cs_hash, ThreadID thread_id);

 private:
  Mutex mutex_;
  std::map<int32_t, std::map<uint64_t, orbit_client_protos::CallstackEvent>> callstack_events_;
  std::atomic<uint64_t> min_time_;
  std::atomic<uint64_t> max_time_;
};

#endif  // ORBIT_CORE_EVENT_BUFFER_H_
