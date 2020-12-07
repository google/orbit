// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/TracepointData.h"

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"

using orbit_client_protos::TracepointEventInfo;

void TracepointData::EmplaceTracepointEvent(uint64_t time, uint64_t tracepoint_hash,
                                            int32_t process_id, int32_t thread_id, int32_t cpu,
                                            bool is_same_pid_as_target) {
  absl::MutexLock lock(&mutex_);
  num_total_tracepoints_++;

  orbit_client_protos::TracepointEventInfo event;
  event.set_time(time);
  CHECK(HasTracepointKey(tracepoint_hash));
  event.set_tracepoint_info_key(tracepoint_hash);
  event.set_tid(thread_id);
  event.set_pid(process_id);
  event.set_cpu(cpu);

  int32_t insertion_thread_id =
      (is_same_pid_as_target) ? thread_id : orbit_base::kNotTargetProcessTid;

  auto [event_map_iterator, unused_inserted] =
      thread_id_to_time_to_tracepoint_.try_emplace(insertion_thread_id);
  auto [unused_iterator, event_inserted] =
      event_map_iterator->second.try_emplace(time, std::move(event));
  if (!event_inserted) {
    ERROR(
        "Tracepoint event was not inserted as there was already an event on this time and "
        "thread.");
  }
}

[[nodiscard]] const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>&
TracepointData::GetTracepointsOfThread(int32_t thread_id) const {
  absl::MutexLock lock(&mutex_);
  static std::map<uint64_t, orbit_client_protos::TracepointEventInfo> empty;
  const auto& it = thread_id_to_time_to_tracepoint_.find(thread_id);
  if (it == thread_id_to_time_to_tracepoint_.end()) {
    return empty;
  }
  return it->second;
}

void TracepointData::ForEachTracepointEvent(
    const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const {
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
    const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& time_to_tracepoint_events,
    const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) {
  for (auto time_to_tracepoint_event = time_to_tracepoint_events.lower_bound(min_tick);
       time_to_tracepoint_event->first < max_tick_exclusive &&
       (time_to_tracepoint_event != time_to_tracepoint_events.end());
       ++time_to_tracepoint_event) {
    action(time_to_tracepoint_event->second);
  }
}
}  // namespace

void TracepointData::ForEachTracepointEventOfThreadInTimeRange(
    int32_t thread_id, uint64_t min_tick, uint64_t max_tick_exclusive,
    const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const {
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

uint32_t TracepointData::GetNumTracepointsForThreadId(int32_t thread_id) const {
  absl::MutexLock lock(&mutex_);
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    return num_total_tracepoints_;
  }
  if (thread_id == orbit_base::kAllProcessThreadsTid) {
    const auto not_target_process_tracepoints_it =
        thread_id_to_time_to_tracepoint_.find(orbit_base::kNotTargetProcessTid);
    if (not_target_process_tracepoints_it == thread_id_to_time_to_tracepoint_.end()) {
      return num_total_tracepoints_;
    }
    return num_total_tracepoints_ - not_target_process_tracepoints_it->second.size();
  }

  const auto& it = thread_id_to_time_to_tracepoint_.find(thread_id);
  if (it == thread_id_to_time_to_tracepoint_.end()) {
    return 0;
  }
  return it->second.size();
}

bool TracepointData::AddUniqueTracepointInfo(uint64_t key,
                                             orbit_grpc_protos::TracepointInfo tracepoint) {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto [unused_it, inserted] = unique_tracepoints_.try_emplace(key, tracepoint);
  return inserted;
}

orbit_grpc_protos::TracepointInfo TracepointData::GetTracepointInfo(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto it = unique_tracepoints_.find(key);
  if (it != unique_tracepoints_.end()) {
    return it->second;
  }
  return {};
}

bool TracepointData::HasTracepointKey(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  return unique_tracepoints_.contains(key);
}
