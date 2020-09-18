// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallstackData.h"

#include "Callstack.h"
#include "SamplingProfiler.h"

using orbit_client_protos::CallstackEvent;

void CallstackData::AddCallstackEvent(CallstackEvent callstack_event) {
  CallstackID hash = callstack_event.callstack_hash();
  CHECK(HasCallStack(hash));

  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  RegisterTime(callstack_event.time());
  callstack_events_by_tid_[callstack_event.thread_id()][callstack_event.time()] =
      std::move(callstack_event);
}

void CallstackData::RegisterTime(uint64_t time) {
  if (time > max_time_) max_time_ = time;
  if (time > 0 && time < min_time_) min_time_ = time;
}

void CallstackData::AddUniqueCallStack(CallStack call_stack) {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  CallstackID hash = call_stack.GetHash();
  unique_callstacks_[hash] = std::make_shared<CallStack>(std::move(call_stack));
}

uint32_t CallstackData::GetCallstackEventsCount() const {
  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  uint32_t count = 0;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    count += tid_and_events.second.size();
  }
  return count;
}

absl::flat_hash_map<int32_t, uint32_t> CallstackData::GetCallstackEventsCountsPerTid() const {
  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  absl::flat_hash_map<int32_t, uint32_t> counts;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    counts.emplace(tid_and_events.first, tid_and_events.second.size());
  }
  return counts;
}

uint32_t CallstackData::GetCallstackEventsOfTidCount(int32_t thread_id) const {
  if (thread_id == SamplingProfiler::kAllThreadsFakeTid) {
    return GetCallstackEventsCount();
  }

  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(thread_id);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return 0;
  }
  return tid_and_events_it->second.size();
}

std::vector<CallstackEvent> CallstackData::GetCallstackEventsOfTidInTimeRange(
    int32_t desired_tid, uint64_t time_begin, uint64_t time_end) const {
  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  std::vector<CallstackEvent> callstack_events;
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    const int32_t tid = tid_and_events.first;
    if (desired_tid != SamplingProfiler::kAllThreadsFakeTid && tid != desired_tid) {
      continue;
    }
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

void CallstackData::ForEachCallstackEvent(
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  for (const auto& tid_and_events : callstack_events_by_tid_) {
    for (const auto& time_and_event : tid_and_events.second) {
      action(time_and_event.second);
    }
  }
}

void CallstackData::ForEachCallstackEventOfTid(
    int32_t desired_tid,
    const std::function<void(const orbit_client_protos::CallstackEvent&)>& action) const {
  if (desired_tid == SamplingProfiler::kAllThreadsFakeTid) {
    ForEachCallstackEvent(action);
    return;
  }

  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  const auto& tid_and_events_it = callstack_events_by_tid_.find(desired_tid);
  if (tid_and_events_it == callstack_events_by_tid_.end()) {
    return;
  }
  for (const auto& time_and_event : tid_and_events_it->second) {
    action(time_and_event.second);
  }
}

void CallstackData::AddCallStackFromKnownCallstackData(const CallstackEvent& event,
                                                       const CallstackData* known_callstack_data) {
  uint64_t hash = event.callstack_hash();
  std::shared_ptr<CallStack> unique_callstack = known_callstack_data->GetCallstackPtr(hash);
  if (unique_callstack == nullptr) {
    return;
  }

  if (!HasCallStack(hash)) {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    unique_callstacks_[hash] = std::move(unique_callstack);
  }
  absl::MutexLock lock(&callstack_events_by_tid_mutex_);
  callstack_events_by_tid_[event.thread_id()][event.time()] = CallstackEvent(event);
}

const CallStack* CallstackData::GetCallStack(CallstackID callstack_id) const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool CallstackData::HasCallStack(CallstackID callstack_id) const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  return unique_callstacks_.count(callstack_id) > 0;
}

void CallstackData::ForEachUniqueCallstack(
    const std::function<void(const CallStack&)>& action) const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  for (const auto& it : unique_callstacks_) {
    action(*it.second);
  }
}

void CallstackData::ForEachFrameInCallstack(uint64_t callstack_id,
                                            const std::function<void(uint64_t)>& action) const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  for (uint64_t frame : unique_callstacks_.at(callstack_id)->GetFrames()) {
    action(frame);
  }
}

absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>>
CallstackData::GetUniqueCallstacksCopy() const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  return unique_callstacks_;
}

std::shared_ptr<CallStack> CallstackData::GetCallstackPtr(CallstackID callstack_id) const {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  auto it = unique_callstacks_.find(callstack_id);
  if (it != unique_callstacks_.end()) {
    return unique_callstacks_.at(callstack_id);
  }
  return nullptr;
}
