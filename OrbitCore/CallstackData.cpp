// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallstackData.h"

#include "Callstack.h"

using orbit_client_protos::CallstackEvent;

void CallstackData::AddCallstackEvent(CallstackEvent callstack_event) {
  std::lock_guard lock(mutex_);
  CallstackID hash = callstack_event.callstack_hash();
  CHECK(unique_callstacks_.contains(hash));
  RegisterTime(callstack_event.time());
  callstack_events_by_tid_[callstack_event.thread_id()][callstack_event.time()] =
      std::move(callstack_event);
}

void CallstackData::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

void CallstackData::AddUniqueCallStack(CallStack call_stack) {
  std::lock_guard lock(mutex_);
  CallstackID hash = call_stack.GetHash();
  unique_callstacks_[hash] = std::make_shared<CallStack>(std::move(call_stack));
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
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    for (const auto& time_and_event : tid_and_events.second) {
      action(time_and_event.second);
    }
  }
}

void CallstackData::ForEachCallstackEventOfTid(
    int32_t tid,
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  std::lock_guard lock(mutex_);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(tid);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return;
  }
  for (const auto& time_and_event : tid_and_events_it->second) {
    action(time_and_event.second);
  }
}

void CallstackData::AddCallStackFromKnownCallstackData(const CallstackEvent& event,
                                                       const CallstackData* known_callstack_data) {
  std::lock_guard lock(mutex_);
  uint64_t hash = event.callstack_hash();
  std::shared_ptr<CallStack> unique_callstack = known_callstack_data->GetCallstackPtr(hash);
  if (unique_callstack == nullptr) {
    return;
  }

  // The insertion only happens if the hash isn't already present.
  unique_callstacks_.emplace(hash, std::move(unique_callstack));
  callstack_events_by_tid_[event.thread_id()][event.time()] = CallstackEvent(event);
}

const CallStack* CallstackData::GetCallStack(CallstackID callstack_id) const {
  std::lock_guard lock(mutex_);
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool CallstackData::HasCallStack(CallstackID callstack_id) const {
  std::lock_guard lock(mutex_);
  return unique_callstacks_.contains(callstack_id);
}

void CallstackData::ForEachUniqueCallstack(
    const std::function<void(const CallStack&)>& action) const {
  std::lock_guard lock(mutex_);
  for (const auto& it : unique_callstacks_) {
    action(*it.second);
  }
}

void CallstackData::ForEachFrameInCallstack(uint64_t callstack_id,
                                            const std::function<void(uint64_t)>& action) const {
  std::lock_guard lock(mutex_);
  for (uint64_t frame : unique_callstacks_.at(callstack_id)->GetFrames()) {
    action(frame);
  }
}

absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>>
CallstackData::GetUniqueCallstacksCopy() const {
  std::lock_guard lock(mutex_);
  return unique_callstacks_;
}

std::shared_ptr<CallStack> CallstackData::GetCallstackPtr(CallstackID callstack_id) const {
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return unique_callstacks_.at(callstack_id);
  }
  return nullptr;
}
