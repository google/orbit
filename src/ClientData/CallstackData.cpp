// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CallstackData.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <utility>

#include "ClientData/CallstackTypes.h"
#include "OrbitBase/Logging.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;

namespace orbit_client_data {

void CallstackData::AddCallstackEvent(CallstackEvent callstack_event) {
  std::lock_guard lock(mutex_);
  CHECK(unique_callstacks_.contains(callstack_event.callstack_id()));
  RegisterTime(callstack_event.time());
  callstack_events_by_tid_[callstack_event.thread_id()][callstack_event.time()] =
      std::move(callstack_event);
}

void CallstackData::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

void CallstackData::AddUniqueCallstack(uint64_t callstack_id,
                                       orbit_client_protos::CallstackInfo callstack) {
  std::lock_guard lock(mutex_);
  unique_callstacks_[callstack_id] =
      std::make_shared<orbit_client_protos::CallstackInfo>(std::move(callstack));
}

uint32_t CallstackData::GetCallstackEventsCount() const {
  std::lock_guard lock(mutex_);
  uint32_t count = 0;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    count += tid_and_events.second.size();
  }
  return count;
}

std::vector<orbit_client_protos::CallstackEvent> CallstackData::GetCallstackEventsInTimeRange(
    uint64_t time_begin, uint64_t time_end) const {
  std::lock_guard lock(mutex_);
  std::vector<CallstackEvent> callstack_events;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    const std::map<uint64_t, CallstackEvent>& events = tid_and_events.second;
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

absl::flat_hash_map<int32_t, uint32_t> CallstackData::GetCallstackEventsCountsPerTid() const {
  std::lock_guard lock(mutex_);
  absl::flat_hash_map<int32_t, uint32_t> counts;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    counts.emplace(tid_and_events.first, tid_and_events.second.size());
  }
  return counts;
}

uint32_t CallstackData::GetCallstackEventsOfTidCount(int32_t thread_id) const {
  std::lock_guard lock(mutex_);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(thread_id);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return 0;
  }
  return tid_and_events_it->second.size();
}

std::vector<CallstackEvent> CallstackData::GetCallstackEventsOfTidInTimeRange(
    int32_t tid, uint64_t time_begin, uint64_t time_end) const {
  std::lock_guard lock(mutex_);
  std::vector<CallstackEvent> callstack_events;

  auto tid_and_events_it = callstack_events_by_tid_.find(tid);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return callstack_events;
  }

  const std::map<uint64_t, CallstackEvent>& events = tid_and_events_it->second;
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

void CallstackData::ForEachCallstackEvent(
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  std::lock_guard lock(mutex_);
  for (const auto& [unused_tid, events] : callstack_events_by_tid_) {
    for (const auto& [unused_timestamp, event] : events) {
      action(event);
    }
  }
}

void CallstackData::ForEachCallstackEventInTimeRange(
    uint64_t min_timestamp, uint64_t max_timestamp,
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  std::lock_guard lock(mutex_);
  CHECK(min_timestamp <= max_timestamp);
  for (const auto& [unused_tid, events] : callstack_events_by_tid_) {
    for (auto event_it = events.lower_bound(min_timestamp);
         event_it != events.upper_bound(max_timestamp); ++event_it) {
      action(event_it->second);
    }
  }
}

void CallstackData::ForEachCallstackEventOfTidInTimeRange(
    int32_t tid, uint64_t min_timestamp, uint64_t max_timestamp,
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  std::lock_guard lock(mutex_);
  CHECK(min_timestamp <= max_timestamp);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(tid);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return;
  }
  const auto& events = tid_and_events_it->second;
  for (auto event_it = events.lower_bound(min_timestamp);
       event_it != events.upper_bound(max_timestamp); ++event_it) {
    action(event_it->second);
  }
}

void CallstackData::AddCallstackFromKnownCallstackData(const CallstackEvent& event,
                                                       const CallstackData* known_callstack_data) {
  std::lock_guard lock(mutex_);
  uint64_t callstack_id = event.callstack_id();
  std::shared_ptr<orbit_client_protos::CallstackInfo> unique_callstack =
      known_callstack_data->GetCallstackPtr(callstack_id);
  if (unique_callstack == nullptr) {
    return;
  }

  // The insertion only happens if the hash isn't already present.
  unique_callstacks_.emplace(callstack_id, std::move(unique_callstack));
  callstack_events_by_tid_[event.thread_id()][event.time()] = CallstackEvent(event);
}

const orbit_client_protos::CallstackInfo* CallstackData::GetCallstack(uint64_t callstack_id) const {
  std::lock_guard lock(mutex_);
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool CallstackData::HasCallstack(uint64_t callstack_id) const {
  std::lock_guard lock(mutex_);
  return unique_callstacks_.contains(callstack_id);
}

void CallstackData::ForEachUniqueCallstack(
    const std::function<void(uint64_t callstack_id,
                             const orbit_client_protos::CallstackInfo& callstack)>& action) const {
  std::lock_guard lock(mutex_);
  for (const auto& [callstack_id, callstack_ptr] : unique_callstacks_) {
    action(callstack_id, *callstack_ptr);
  }
}

void CallstackData::ForEachFrameInCallstack(uint64_t callstack_id,
                                            const std::function<void(uint64_t)>& action) const {
  std::lock_guard lock(mutex_);
  for (uint64_t frame : unique_callstacks_.at(callstack_id)->frames()) {
    action(frame);
  }
}

absl::flat_hash_map<uint64_t, std::shared_ptr<orbit_client_protos::CallstackInfo>>
CallstackData::GetUniqueCallstacksCopy() const {
  std::lock_guard lock(mutex_);
  return unique_callstacks_;
}

std::shared_ptr<orbit_client_protos::CallstackInfo> CallstackData::GetCallstackPtr(
    uint64_t callstack_id) const {
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return unique_callstacks_.at(callstack_id);
  }
  return nullptr;
}

void CallstackData::UpdateCallstackTypeBasedOnMajorityStart() {
  std::lock_guard lock(mutex_);

  absl::flat_hash_set<uint64_t> callstack_ids_to_filter;

  for (auto& [tid, timestamps_and_callstack_events] : callstack_events_by_tid_) {
    uint64_t count_for_this_thread = 0;

    // Count the number of occurrences of each outer frame for this thread.
    absl::flat_hash_map<uint64_t, uint64_t> count_by_outer_frame;
    for (const auto& [unused_timestamp_ns, event] : timestamps_and_callstack_events) {
      const CallstackInfo& callstack = *unique_callstacks_.at(event.callstack_id());
      CHECK(callstack.type() != CallstackInfo::kFilteredByMajorityOutermostFrame);
      if (callstack.type() != CallstackInfo::kComplete) {
        continue;
      }
      ++count_for_this_thread;

      const auto& frames = callstack.frames();
      CHECK(!frames.empty());
      uint64_t outer_frame = *frames.rbegin();
      ++count_by_outer_frame[outer_frame];
    }

    // Find the outer frame with the most occurrences.
    if (count_by_outer_frame.empty()) {
      continue;
    }
    uint64_t majority_outer_frame = 0;
    uint64_t majority_outer_frame_count = 0;
    for (const auto& outer_frame_and_count : count_by_outer_frame) {
      CHECK(outer_frame_and_count.second > 0);
      if (outer_frame_and_count.second > majority_outer_frame_count) {
        majority_outer_frame = outer_frame_and_count.first;
        majority_outer_frame_count = outer_frame_and_count.second;
      }
    }

    // The value is somewhat arbitrary. We want at least three quarters of the thread's callstacks
    // to agree on the "correct" outermost frame.
    static constexpr double kFilterSupermajorityThreshold = 0.75;
    if (majority_outer_frame_count < count_for_this_thread * kFilterSupermajorityThreshold) {
      LOG("Skipping filtering CallstackEvents for tid %d: majority outer frame has only %lu "
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
      CHECK(callstack.type() != CallstackInfo::kFilteredByMajorityOutermostFrame);
      if (callstack.type() != CallstackInfo::kComplete) {
        continue;
      }

      const auto& frames = unique_callstacks_.at(event.callstack_id())->frames();
      CHECK(!frames.empty());
      if (*frames.rbegin() != majority_outer_frame) {
        callstack_ids_to_filter.insert(event.callstack_id());
      }
    }
  }

  // Change the type of the recorded CallstackInfos.
  for (uint64_t callstack_id_to_filter : callstack_ids_to_filter) {
    CallstackInfo* callstack = unique_callstacks_.at(callstack_id_to_filter).get();
    CHECK(callstack->type() == CallstackInfo::kComplete);
    callstack->set_type(CallstackInfo::kFilteredByMajorityOutermostFrame);
  }

  // Count how many CallstackEvents had their CallstackInfo affected by the type change.
  uint64_t affected_event_count = 0;
  for (auto& [tid, timestamps_and_callstack_events] : callstack_events_by_tid_) {
    for (const auto& [unused_timestamp_ns, event] : timestamps_and_callstack_events) {
      if (unique_callstacks_.at(event.callstack_id())->type() ==
          CallstackInfo::kFilteredByMajorityOutermostFrame) {
        ++affected_event_count;
      }
    }
  }

  uint32_t callstack_event_count = GetCallstackEventsCount();
  LOG("Filtered %u CallstackInfos of %u (%.2f%%), affecting %u CallstackEvents of %u (%.2f%%)",
      callstack_ids_to_filter.size(), unique_callstacks_.size(),
      100.0f * callstack_ids_to_filter.size() / unique_callstacks_.size(), affected_event_count,
      callstack_event_count, 100.0f * affected_event_count / callstack_event_count);
}

}  // namespace orbit_client_data
