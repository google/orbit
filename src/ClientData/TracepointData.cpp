// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TracepointData.h"

#include <absl/meta/type_traits.h>

#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"

namespace orbit_client_data {

void TracepointData::EmplaceTracepointEvent(uint64_t timestamp_ns, uint64_t tracepoint_id,
                                            uint32_t process_id, uint32_t thread_id, int32_t cpu,
                                            bool is_same_pid_as_target) {
  absl::MutexLock lock(&mutex_);
  num_total_tracepoint_events_++;

  TracepointEventInfo event(process_id, thread_id, cpu, timestamp_ns, tracepoint_id);

  int32_t insertion_thread_id =
      (is_same_pid_as_target) ? thread_id : orbit_base::kNotTargetProcessTid;

  auto [event_map_iterator, unused_inserted] =
      thread_id_to_time_to_tracepoint_.try_emplace(insertion_thread_id);
  auto [unused_iterator, event_inserted] =
      event_map_iterator->second.try_emplace(timestamp_ns, std::move(event));
  if (!event_inserted) {
    ORBIT_ERROR(
        "Tracepoint event was not inserted as there was already an event on this timestamp_ns and "
        "thread.");
  }
}

void TracepointData::ForEachTracepointEvent(
    const std::function<void(const TracepointEventInfo&)>& action) const {
  absl::MutexLock lock(&mutex_);
  for (auto const& entry : thread_id_to_time_to_tracepoint_) {
    for (auto const& time_to_tracepoint_event : entry.second) {
      action(time_to_tracepoint_event.second);
    }
  }
}

namespace {
void ForEachTracepointEventInRange(
    uint64_t min_tick, uint64_t max_tick_exclusive,
    const std::map<uint64_t, TracepointEventInfo>& time_to_tracepoint_events,
    const std::function<void(const TracepointEventInfo&)>& action) {
  for (auto time_to_tracepoint_event = time_to_tracepoint_events.lower_bound(min_tick);
       time_to_tracepoint_event != time_to_tracepoint_events.end() &&
       time_to_tracepoint_event->first < max_tick_exclusive;
       ++time_to_tracepoint_event) {
    action(time_to_tracepoint_event->second);
  }
}
}  // namespace

void TracepointData::ForEachTracepointEventOfThreadInTimeRange(
    uint32_t thread_id, uint64_t min_tick, uint64_t max_tick_exclusive,
    const std::function<void(const TracepointEventInfo&)>& action) const {
  absl::MutexLock lock(&mutex_);
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    for (const auto& [unused_thread_id, time_to_tracepoint] : thread_id_to_time_to_tracepoint_) {
      ForEachTracepointEventInRange(min_tick, max_tick_exclusive, time_to_tracepoint, action);
    }
  } else if (thread_id == orbit_base::kAllProcessThreadsTid) {
    for (const auto& [thread_id, time_to_tracepoint] : thread_id_to_time_to_tracepoint_) {
      if (thread_id == orbit_base::kNotTargetProcessTid) {
        continue;
      }
      ForEachTracepointEventInRange(min_tick, max_tick_exclusive, time_to_tracepoint, action);
    }
  } else {
    const auto& it = thread_id_to_time_to_tracepoint_.find(thread_id);
    if (it == thread_id_to_time_to_tracepoint_.end()) {
      return;
    }
    ForEachTracepointEventInRange(min_tick, max_tick_exclusive, it->second, action);
  }
}

uint32_t TracepointData::GetNumTracepointEventsForThreadId(uint32_t thread_id) const {
  absl::MutexLock lock(&mutex_);
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    return num_total_tracepoint_events_;
  }
  if (thread_id == orbit_base::kAllProcessThreadsTid) {
    const auto not_target_process_tracepoints_it =
        thread_id_to_time_to_tracepoint_.find(orbit_base::kNotTargetProcessTid);
    if (not_target_process_tracepoints_it == thread_id_to_time_to_tracepoint_.end()) {
      return num_total_tracepoint_events_;
    }
    return num_total_tracepoint_events_ - not_target_process_tracepoints_it->second.size();
  }

  const auto& it = thread_id_to_time_to_tracepoint_.find(thread_id);
  if (it == thread_id_to_time_to_tracepoint_.end()) {
    return 0;
  }
  return it->second.size();
}

bool TracepointData::AddUniqueTracepointInfo(uint64_t key, TracepointInfo tracepoint) {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto [unused_it, inserted] =
      unique_tracepoints_.try_emplace(key, std::make_unique<TracepointInfo>(std::move(tracepoint)));
  return inserted;
}

const TracepointInfo* TracepointData::GetTracepointInfo(uint64_t tracepoint_id) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto it = unique_tracepoints_.find(tracepoint_id);
  if (it != unique_tracepoints_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool TracepointData::HasTracepointId(uint64_t tracepoint_id) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  return unique_tracepoints_.contains(tracepoint_id);
}

void TracepointData::ForEachUniqueTracepointInfo(
    const std::function<void(const TracepointInfo&)>& action) const {
  absl::MutexLock lock(&unique_tracepoints_mutex_);
  for (const auto& it : unique_tracepoints_) {
    ORBIT_CHECK(it.second != nullptr);
    action(*it.second);
  }
}

}  // namespace orbit_client_data
