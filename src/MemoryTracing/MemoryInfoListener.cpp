// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracing/MemoryInfoListener.h"

#include <absl/types/span.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace orbit_memory_tracing {

[[nodiscard]] static uint64_t GetSamplingWindowId(uint64_t sample_timestamp_ns,
                                                  uint64_t sampling_start_timestamp_ns,
                                                  uint64_t sampling_period_ns) {
  return (sample_timestamp_ns - sampling_start_timestamp_ns + (sampling_period_ns / 2)) /
         sampling_period_ns;
}

// This method computes the arithmetic mean of input timestamps.
[[nodiscard]] static uint64_t GetSynchronizedSamplingTimestamp(
    absl::Span<const uint64_t> sampling_timestamps) {
  uint64_t offset = *std::min_element(sampling_timestamps.begin(), sampling_timestamps.end());
  uint64_t sum = 0;
  for (uint64_t timestamp : sampling_timestamps) sum += timestamp - offset;
  return sum / sampling_timestamps.size() + offset;
}

void MemoryInfoListener::ProcessMemoryUsageEventIfReady(uint64_t sampling_window_id) {
  if (!in_progress_memory_usage_events_.contains(sampling_window_id)) return;

  orbit_grpc_protos::MemoryUsageEvent& memory_usage_event =
      in_progress_memory_usage_events_[sampling_window_id];
  bool memory_usage_event_is_ready = memory_usage_event.has_system_memory_usage();
  if (enable_cgroup_memory_) {
    memory_usage_event_is_ready =
        memory_usage_event_is_ready && memory_usage_event.has_cgroup_memory_usage();
  }
  if (enable_process_memory_) {
    memory_usage_event_is_ready =
        memory_usage_event_is_ready && memory_usage_event.has_process_memory_usage();
  }
  if (!memory_usage_event_is_ready) return;

  std::vector<uint64_t> timestamps{memory_usage_event.system_memory_usage().timestamp_ns()};
  if (enable_cgroup_memory_) {
    timestamps.push_back(memory_usage_event.cgroup_memory_usage().timestamp_ns());
  }
  if (enable_process_memory_) {
    timestamps.push_back(memory_usage_event.process_memory_usage().timestamp_ns());
  }
  uint64_t synchronized_timestamp_ns = GetSynchronizedSamplingTimestamp(timestamps);
  memory_usage_event.set_timestamp_ns(synchronized_timestamp_ns);
  OnMemoryUsageEvent(std::move(memory_usage_event));

  in_progress_memory_usage_events_.erase(sampling_window_id);
}

void MemoryInfoListener::OnSystemMemoryUsage(
    orbit_grpc_protos::SystemMemoryUsage system_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(
      system_memory_usage.timestamp_ns(), sampling_start_timestamp_ns_, sampling_period_ns_);

  absl::MutexLock lock(&in_progress_memory_usage_events_mutex_);
  *in_progress_memory_usage_events_[sampling_window_id].mutable_system_memory_usage() =
      std::move(system_memory_usage);
  ProcessMemoryUsageEventIfReady(sampling_window_id);
}

void MemoryInfoListener::OnCGroupMemoryUsage(
    orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(
      cgroup_memory_usage.timestamp_ns(), sampling_start_timestamp_ns_, sampling_period_ns_);

  absl::MutexLock lock(&in_progress_memory_usage_events_mutex_);
  *in_progress_memory_usage_events_[sampling_window_id].mutable_cgroup_memory_usage() =
      std::move(cgroup_memory_usage);
  ProcessMemoryUsageEventIfReady(sampling_window_id);
}

void MemoryInfoListener::OnProcessMemoryUsage(
    orbit_grpc_protos::ProcessMemoryUsage process_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(
      process_memory_usage.timestamp_ns(), sampling_start_timestamp_ns_, sampling_period_ns_);

  absl::MutexLock lock(&in_progress_memory_usage_events_mutex_);
  *in_progress_memory_usage_events_[sampling_window_id].mutable_process_memory_usage() =
      std::move(process_memory_usage);
  ProcessMemoryUsageEventIfReady(sampling_window_id);
}

}  // namespace orbit_memory_tracing
