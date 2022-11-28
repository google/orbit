// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_DATA_H_
#define CLIENT_DATA_CALLSTACK_DATA_H_

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <stdint.h>

#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "CallstackType.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "FastRenderingUtils.h"
#include "ModuleManager.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

class CallstackData {
 public:
  explicit CallstackData() = default;

  CallstackData(const CallstackData& other) = delete;
  CallstackData& operator=(const CallstackData& other) = delete;
  CallstackData(CallstackData&& other) = delete;
  CallstackData& operator=(CallstackData&& other) = delete;

  ~CallstackData() = default;

  // Assume that callstack_event.callstack_hash is filled correctly and the
  // Callstack with the corresponding id is already in unique_callstacks_.
  void AddCallstackEvent(orbit_client_data::CallstackEvent callstack_event);
  void AddUniqueCallstack(uint64_t callstack_id, CallstackInfo callstack);
  void AddCallstackFromKnownCallstackData(const orbit_client_data::CallstackEvent& event,
                                          const CallstackData& known_callstack_data);

  [[nodiscard]] uint32_t GetCallstackEventsCount() const;

  [[nodiscard]] std::vector<orbit_client_data::CallstackEvent> GetCallstackEventsInTimeRange(
      uint64_t time_begin, uint64_t time_end) const;

  [[nodiscard]] uint32_t GetCallstackEventsOfTidCount(uint32_t thread_id) const;

  [[nodiscard]] std::vector<orbit_client_data::CallstackEvent> GetCallstackEventsOfTidInTimeRange(
      uint32_t tid, uint64_t time_begin, uint64_t time_end) const;

  template <typename Action>
  void ForEachCallstackEvent(Action&& action) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto& [unused_tid, events] : callstack_events_by_tid_) {
      for (const auto& [unused_timestamp, event] : events) {
        std::invoke(action, event);
      }
    }
  }

  template <typename Action>
  void ForEachCallstackEventInTimeRange(uint64_t min_timestamp, uint64_t max_timestamp,
                                        Action&& action) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ORBIT_CHECK(min_timestamp <= max_timestamp);
    for (const auto& [unused_tid, events] : callstack_events_by_tid_) {
      for (auto event_it = events.lower_bound(min_timestamp);
           event_it != events.upper_bound(max_timestamp); ++event_it) {
        std::invoke(action, event_it->second);
      }
    }
  }

  // Do a particular action for callstacks but skipping callstacks that will be rendered later in
  // the same pixel on the screen. It assures to do the action at most once per pixel. This
  // iteration is faster than the non-discretized one since it doesn't require going through all
  // callstacks.
  template <typename Action>
  void ForEachCallstackEventInTimeRangeDiscretized(uint64_t min_timestamp, uint64_t max_timestamp,
                                                   uint32_t resolution, Action&& action) const {
    auto get_next_callstack = [&](uint64_t timestamp) -> std::optional<CallstackEvent> {
      std::optional<CallstackEvent> next_callstack;
      const uint32_t current_pixel =
          GetPixelNumber(timestamp, resolution, min_timestamp, max_timestamp);
      for (const auto& [unused_tid, events] : callstack_events_by_tid_) {
        auto next_callstack_of_tid = events.lower_bound(timestamp);
        if (next_callstack_of_tid == events.end() ||
            (next_callstack.has_value() &&
             next_callstack.value().timestamp_ns() <= next_callstack_of_tid->second.timestamp_ns()))
          continue;

        // If this callstack will be drawn in the current_pixel, we don't need to search for more of
        // them. Otherwise there could be a callstack in another thread_id that will be draw before,
        // so we need to keep looking.
        if (GetPixelNumber(next_callstack_of_tid->first, resolution, min_timestamp,
                           max_timestamp) == current_pixel) {
          return next_callstack_of_tid->second;
        }
        next_callstack = next_callstack_of_tid->second;
      }
      return next_callstack;
    };

    for (std::optional<CallstackEvent> next_callstack = get_next_callstack(min_timestamp);
         next_callstack.has_value() && next_callstack.value().timestamp_ns() < max_timestamp;
         next_callstack = get_next_callstack(GetNextPixelBoundaryTimeNs(
             next_callstack.value().timestamp_ns(), resolution, min_timestamp, max_timestamp))) {
      std::invoke(action, next_callstack.value());
    }
  }

  template <typename Action>
  void ForEachCallstackEventOfTidInTimeRange(uint32_t tid, uint64_t min_timestamp,
                                             uint64_t max_timestamp, Action&& action) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ORBIT_CHECK(min_timestamp <= max_timestamp);
    const auto& tid_and_events_it = callstack_events_by_tid_.find(tid);
    if (tid_and_events_it == callstack_events_by_tid_.end()) {
      return;
    }
    const auto& events = tid_and_events_it->second;
    for (auto event_it = events.lower_bound(min_timestamp);
         event_it != events.upper_bound(max_timestamp); ++event_it) {
      std::invoke(action, event_it->second);
    }
  }

  // Do a particular action for all callstacks in a thread but skipping callstacks that will be
  // rendered later in the same pixel on the screen. It assures to do the action at most once per
  // pixel. This iteration is faster than the non-discretized one since it does not require going
  // through all callstacks.
  template <typename Action>
  void ForEachCallstackEventOfTidInTimeRangeDiscretized(uint32_t tid, uint64_t min_timestamp,
                                                        uint64_t max_timestamp, uint32_t resolution,
                                                        Action&& action) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    const auto& tid_and_events_it = callstack_events_by_tid_.find(tid);
    if (tid_and_events_it == callstack_events_by_tid_.end()) {
      return;
    }
    const auto& events = tid_and_events_it->second;
    for (auto event_it = events.lower_bound(min_timestamp);
         event_it != events.end() && event_it->first < max_timestamp;
         event_it = events.lower_bound(GetNextPixelBoundaryTimeNs(event_it->first, resolution,
                                                                  min_timestamp, max_timestamp))) {
      std::invoke(action, event_it->second);
    }
  }

  [[nodiscard]] uint64_t max_time() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return max_time_;
  }

  [[nodiscard]] uint64_t min_time() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return min_time_;
  }

  [[nodiscard]] const CallstackInfo* GetCallstack(uint64_t callstack_id) const;

  [[nodiscard]] bool HasCallstack(uint64_t callstack_id) const;

  template <typename Action>
  void ForEachUniqueCallstack(Action&& action) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto& [callstack_id, callstack_ptr] : unique_callstacks_) {
      std::invoke(action, callstack_id, *callstack_ptr);
    }
  }

  // Assuming that, for each thread, the outermost frame of each callstack is always the same,
  // update the type of all the kComplete callstacks that have the outermost frame not matching the
  // majority outermost frame. This is a way to filter unwinding errors that were not reported as
  // such.
  void UpdateCallstackTypeBasedOnMajorityStart(
      const std::map<uint64_t, uint64_t>&
          absolute_address_to_size_of_functions_to_stop_unwinding_at);

 private:
  [[nodiscard]] std::shared_ptr<CallstackInfo> GetCallstackPtr(uint64_t callstack_id) const;

  void RegisterTime(uint64_t time);

  // Use a reentrant mutex so that calls to the ForEach... methods can be nested.
  // E.g., one might want to nest ForEachCallstackEvent and ForEachFrameInCallstack.
  mutable std::recursive_mutex mutex_;
  absl::flat_hash_map<uint64_t, std::shared_ptr<CallstackInfo>> unique_callstacks_;
  absl::flat_hash_map<uint32_t, absl::btree_map<uint64_t, CallstackEvent>> callstack_events_by_tid_;

  uint64_t max_time_ = 0;
  uint64_t min_time_ = std::numeric_limits<uint64_t>::max();
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_DATA_H_
