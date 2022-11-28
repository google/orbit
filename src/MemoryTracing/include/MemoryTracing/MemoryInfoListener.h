// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_INFO_LISTENER_H_
#define MEMORY_TRACING_MEMORY_INFO_LISTENER_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>

#include <cstdint>

#include "GrpcProtos/capture.pb.h"

namespace orbit_memory_tracing {

// This class serves as an event listener for the memory events.
class MemoryInfoListener {
 public:
  virtual ~MemoryInfoListener() = default;

  void SetSamplingStartTimestampNs(uint64_t sampling_start_timestamp_ns) {
    sampling_start_timestamp_ns_ = sampling_start_timestamp_ns;
  }
  void SetSamplingPeriodNs(uint64_t sampling_period_ns) {
    sampling_period_ns_ = sampling_period_ns;
  }
  void SetEnableCGroupMemory(bool enable_cgroup_memory) {
    enable_cgroup_memory_ = enable_cgroup_memory;
  }
  void SetEnableProcessMemory(bool enable_process_memory) {
    enable_process_memory_ = enable_process_memory;
  }

  void OnSystemMemoryUsage(orbit_grpc_protos::SystemMemoryUsage system_memory_usage);
  void OnCGroupMemoryUsage(orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage);
  void OnProcessMemoryUsage(orbit_grpc_protos::ProcessMemoryUsage process_memory_usage);

 private:
  void ProcessMemoryUsageEventIfReady(uint64_t sampling_window_id)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(in_progress_memory_usage_events_mutex_);
  virtual void OnMemoryUsageEvent(orbit_grpc_protos::MemoryUsageEvent memory_usage_event) = 0;

  absl::flat_hash_map<uint64_t, orbit_grpc_protos::MemoryUsageEvent>
      in_progress_memory_usage_events_ ABSL_GUARDED_BY(in_progress_memory_usage_events_mutex_);
  absl::Mutex in_progress_memory_usage_events_mutex_;
  uint64_t sampling_start_timestamp_ns_;
  uint64_t sampling_period_ns_;
  bool enable_cgroup_memory_ = false;
  bool enable_process_memory_ = false;
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_INFO_LISTENER_H_