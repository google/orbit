// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CallstackData.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <algorithm>
#include <mutex>
#include <utility>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

void CallstackData::AddCallstackEvent(CallstackEvent callstack_event) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  ORBIT_CHECK(unique_callstacks_.contains(callstack_event.callstack_id()));
  RegisterTime(callstack_event.timestamp_ns());
  callstack_events_by_tid_[callstack_event.thread_id()].emplace(callstack_event.timestamp_ns(),
                                                                callstack_event);
}

void CallstackData::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

void CallstackData::AddUniqueCallstack(uint64_t callstack_id, CallstackInfo callstack) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  unique_callstacks_[callstack_id] = std::make_shared<CallstackInfo>(std::move(callstack));
}

uint32_t CallstackData::GetCallstackEventsCount() const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  uint32_t count = 0;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    count += tid_and_events.second.size();
  }
  return count;
}

std::vector<orbit_client_data::CallstackEvent> CallstackData::GetCallstackEventsInTimeRange(
    uint64_t time_begin, uint64_t time_end) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::vector<CallstackEvent> callstack_events;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    const absl::btree_map<uint64_t, CallstackEvent>& events = tid_and_events.second;
    for (auto event_it = events.lower_bound(time_begin); event_it != events.end(); ++event_it) {
      uint64_t time = event_it->first;
      if (time < time_end) {
        callstack_events.push_back(event_it->second);
      } else {
        break;
      }
    }
  }
  return callstack_events;
}

uint32_t CallstackData::GetCallstackEventsOfTidCount(uint32_t thread_id) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(thread_id);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return 0;
  }
  return tid_and_events_it->second.size();
}

std::vector<CallstackEvent> CallstackData::GetCallstackEventsOfTidInTimeRange(
    uint32_t tid, uint64_t time_begin, uint64_t time_end) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::vector<CallstackEvent> callstack_events;

  auto tid_and_events_it = callstack_events_by_tid_.find(tid);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return callstack_events;
  }

  const absl::btree_map<uint64_t, CallstackEvent>& events = tid_and_events_it->second;
  for (auto event_it = events.lower_bound(time_begin); event_it != events.end(); ++event_it) {
    uint64_t time = event_it->first;
    if (time < time_end) {
      callstack_events.push_back(event_it->second);
    } else {
      break;
    }
  }
  return callstack_events;
}

void CallstackData::AddCallstackFromKnownCallstackData(const CallstackEvent& event,
                                                       const CallstackData& known_callstack_data) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  uint64_t callstack_id = event.callstack_id();
  std::shared_ptr<CallstackInfo> unique_callstack =
      known_callstack_data.GetCallstackPtr(callstack_id);
  if (unique_callstack == nullptr) {
    return;
  }

  // The insertion only happens if the hash isn't already present.
  unique_callstacks_.emplace(callstack_id, std::move(unique_callstack));
  callstack_events_by_tid_[event.thread_id()].emplace(event.timestamp_ns(), event);
}

const CallstackInfo* CallstackData::GetCallstack(uint64_t callstack_id) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool CallstackData::HasCallstack(uint64_t callstack_id) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  return unique_callstacks_.contains(callstack_id);
}

std::shared_ptr<CallstackInfo> CallstackData::GetCallstackPtr(uint64_t callstack_id) const {
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return unique_callstacks_.at(callstack_id);
  }
  return nullptr;
}

static bool IsPcInFunctionsToStopUnwindingAt(
    const std::map<uint64_t, uint64_t>& absolute_address_to_size_of_functions_to_stop_unwinding_at,
    uint64_t pc) {
  auto function_it = absolute_address_to_size_of_functions_to_stop_unwinding_at.upper_bound(pc);
  if (function_it == absolute_address_to_size_of_functions_to_stop_unwinding_at.begin()) {
    return false;
  }

  --function_it;

  uint64_t function_start = function_it->first;
  ORBIT_CHECK(function_start <= pc);
  uint64_t size = function_it->second;
  return (pc < function_start + size);
}

void CallstackData::UpdateCallstackTypeBasedOnMajorityStart(
    const std::map<uint64_t, uint64_t>&
        absolute_address_to_size_of_functions_to_stop_unwinding_at) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);

  absl::flat_hash_set<uint64_t> callstack_ids_to_filter;

  for (auto& [tid, timestamps_and_callstack_events] : callstack_events_by_tid_) {
    uint64_t count_for_this_thread = 0;

    // Count the number of occurrences of each outer frame for this thread.
    absl::flat_hash_map<uint64_t, uint64_t> count_by_outer_frame;
    for (const auto& [unused_timestamp_ns, event] : timestamps_and_callstack_events) {
      const CallstackInfo& callstack = *unique_callstacks_.at(event.callstack_id());
      ORBIT_CHECK(callstack.type() != CallstackType::kFilteredByMajorityOutermostFrame);
      if (callstack.type() != CallstackType::kComplete) {
        continue;
      }

      const auto& frames = callstack.frames();
      ORBIT_CHECK(!frames.empty());
      uint64_t outer_frame = *frames.rbegin();
      if (!IsPcInFunctionsToStopUnwindingAt(
              absolute_address_to_size_of_functions_to_stop_unwinding_at, outer_frame)) {
        ++count_for_this_thread;
        ++count_by_outer_frame[outer_frame];
      }
    }

    // Find the outer frame with the most occurrences.
    if (count_by_outer_frame.empty()) {
      continue;
    }
    uint64_t majority_outer_frame = 0;
    uint64_t majority_outer_frame_count = 0;
    for (const auto& outer_frame_and_count : count_by_outer_frame) {
      ORBIT_CHECK(outer_frame_and_count.second > 0);
      if (outer_frame_and_count.second > majority_outer_frame_count) {
        majority_outer_frame = outer_frame_and_count.first;
        majority_outer_frame_count = outer_frame_and_count.second;
      }
    }

    // The value is somewhat arbitrary. We want at least three quarters of the thread's callstacks
    // to agree on the "correct" outermost frame.
    static constexpr double kFilterSupermajorityThreshold = 0.75;
    if (majority_outer_frame_count < count_for_this_thread * kFilterSupermajorityThreshold) {
      ORBIT_LOG(
          "Skipping filtering CallstackEvents for tid %d: majority outer frame has only %lu "
          "occurrences out of %lu",
          tid, majority_outer_frame_count, count_for_this_thread);
      continue;
    }

    // Record the ids of the CallstackInfos references by the CallstackEvents whose outer frame
    // doesn't match the (super)majority outer frame.
    // Note that if a CallstackEvent from another thread references a filtered CallstackInfo, that
    // CallstackEvent will also be affected.
    for (const auto& [unused_timestamp_ns, event] : timestamps_and_callstack_events) {
      const CallstackInfo& callstack = *unique_callstacks_.at(event.callstack_id());
      ORBIT_CHECK(callstack.type() != CallstackType::kFilteredByMajorityOutermostFrame);
      if (callstack.type() != CallstackType::kComplete) {
        continue;
      }

      const auto& frames = unique_callstacks_.at(event.callstack_id())->frames();
      ORBIT_CHECK(!frames.empty());
      uint64_t outermost_frame = *frames.rbegin();
      if (outermost_frame != majority_outer_frame &&
          !IsPcInFunctionsToStopUnwindingAt(
              absolute_address_to_size_of_functions_to_stop_unwinding_at, outermost_frame)) {
        callstack_ids_to_filter.insert(event.callstack_id());
      }
    }
  }

  // Change the type of the recorded CallstackInfos.
  for (uint64_t callstack_id_to_filter : callstack_ids_to_filter) {
    CallstackInfo* callstack = unique_callstacks_.at(callstack_id_to_filter).get();
    ORBIT_CHECK(callstack->type() == CallstackType::kComplete);
    callstack->set_type(CallstackType::kFilteredByMajorityOutermostFrame);
  }

  // Count how many CallstackEvents had their CallstackInfo affected by the type change.
  uint64_t affected_event_count = 0;
  for (auto& [tid, timestamps_and_callstack_events] : callstack_events_by_tid_) {
    for (const auto& [unused_timestamp_ns, event] : timestamps_and_callstack_events) {
      if (unique_callstacks_.at(event.callstack_id())->type() ==
          CallstackType::kFilteredByMajorityOutermostFrame) {
        ++affected_event_count;
      }
    }
  }

  uint32_t callstack_event_count = GetCallstackEventsCount();
  ORBIT_LOG(
      "Filtered %u CallstackInfos of %u (%.2f%%), affecting %u CallstackEvents of %u (%.2f%%)",
      callstack_ids_to_filter.size(), unique_callstacks_.size(),
      100.0f * callstack_ids_to_filter.size() / unique_callstacks_.size(), affected_event_count,
      callstack_event_count, 100.0f * affected_event_count / callstack_event_count);
}

}  // namespace orbit_client_data
