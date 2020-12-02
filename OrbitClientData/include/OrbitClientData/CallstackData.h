// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CALLSTACK_DATA_H_
#define ORBIT_CORE_CALLSTACK_DATA_H_

#include <memory>
#include <mutex>

#include "Callstack.h"
#include "CallstackTypes.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class CallstackData {
 public:
  explicit CallstackData() = default;

  CallstackData(const CallstackData& other) = delete;
  CallstackData& operator=(const CallstackData& other) = delete;
  CallstackData(CallstackData&& other) = delete;
  CallstackData& operator=(CallstackData&& other) = delete;

  ~CallstackData() = default;

  // Assume that callstack_event.callstack_hash is filled correctly and the
  // CallStack with corresponding hash is already in unique_callstacks_
  void AddCallstackEvent(orbit_client_protos::CallstackEvent callstack_event);
  void AddUniqueCallStack(CallStack call_stack);
  void AddCallStackFromKnownCallstackData(const orbit_client_protos::CallstackEvent& event,
                                          const CallstackData* known_callstack_data);

  [[nodiscard]] const absl::flat_hash_map<int32_t,
                                          std::map<uint64_t, orbit_client_protos::CallstackEvent>>&
  callstack_events_by_tid() const {
    return callstack_events_by_tid_;
  }

  [[nodiscard]] uint32_t GetCallstackEventsCount() const;

  [[nodiscard]] std::vector<orbit_client_protos::CallstackEvent> GetCallstackEventsInTimeRange(
      uint64_t time_begin, uint64_t time_end) const;

  [[nodiscard]] absl::flat_hash_map<int32_t, uint32_t> GetCallstackEventsCountsPerTid() const;

  [[nodiscard]] uint32_t GetCallstackEventsOfTidCount(int32_t thread_id) const;

  [[nodiscard]] std::vector<orbit_client_protos::CallstackEvent> GetCallstackEventsOfTidInTimeRange(
      int32_t tid, uint64_t time_begin, uint64_t time_end) const;

  void ForEachCallstackEvent(
      const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const;

  void ForEachCallstackEventOfTid(
      int32_t tid,
      const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const;

  [[nodiscard]] uint64_t max_time() const {
    std::lock_guard lock(mutex_);
    return max_time_;
  }

  [[nodiscard]] uint64_t min_time() const {
    std::lock_guard lock(mutex_);
    return min_time_;
  }

  [[nodiscard]] const CallStack* GetCallStack(CallstackID callstack_id) const;

  [[nodiscard]] bool HasCallStack(CallstackID callstack_id) const;

  void ForEachUniqueCallstack(const std::function<void(const CallStack&)>& action) const;

  void ForEachFrameInCallstack(uint64_t callstack_id,
                               const std::function<void(uint64_t)>& action) const;

  [[nodiscard]] absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>>
  GetUniqueCallstacksCopy() const;

  // Assuming that, for each thread, the outermost frame of each callstack is always the same,
  // filters out all the callstacks that have the outermost frame not matching the majority
  // outermost frame. This is a way to filter unwinding errors that were not reported as such.
  void FilterCallstackEventsBasedOnMajorityStart();

 private:
  [[nodiscard]] std::shared_ptr<CallStack> GetCallstackPtr(CallstackID callstack_id) const;

  void RegisterTime(uint64_t time);

  // Use a reentrant mutex so that calls to the ForEach... methods can be nested.
  // E.g., one might want to nest ForEachCallstackEvent and ForEachFrameInCallstack.
  mutable std::recursive_mutex mutex_;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks_;
  absl::flat_hash_map<int32_t, std::map<uint64_t, orbit_client_protos::CallstackEvent>>
      callstack_events_by_tid_;

  uint64_t max_time_ = 0;
  uint64_t min_time_ = std::numeric_limits<uint64_t>::max();
};

#endif  // ORBIT_CORE_CALLSTACK_DATA_H_
