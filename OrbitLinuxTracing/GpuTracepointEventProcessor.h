// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR
#define ORBIT_LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR

#include <tuple>

#include "OrbitLinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

class GpuTracepointEventProcessor {
 public:
  void PushEvent(const AmdgpuCsIoctlPerfEvent& sample);
  void PushEvent(const AmdgpuSchedRunJobPerfEvent& sample);
  void PushEvent(const DmaFenceSignaledPerfEvent& sample);

  void SetListener(TracerListener* listener);

 private:
  // Keys are context, seqno, and timeline
  typedef std::tuple<uint32_t, uint32_t, std::string> Key;

  int ComputeDepthForEvent(const std::string& timeline, uint64_t start_timestamp,
                           uint64_t end_timestamp);

  void CreateGpuExecutionEventIfComplete(const Key& key);

  TracerListener* listener_ = nullptr;

  struct AmdgpuCsIoctlEvent {
    pid_t tid;
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, AmdgpuCsIoctlEvent> amdgpu_cs_ioctl_events_;

  struct AmdgpuSchedRunJobEvent {
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, AmdgpuSchedRunJobEvent> amdgpu_sched_run_job_events_;

  struct DmaFenceSignaledEvent {
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, DmaFenceSignaledEvent> dma_fence_signaled_events_;

  absl::flat_hash_map<std::string, uint64_t> timeline_to_latest_dma_signal_;

  absl::flat_hash_map<std::string, std::vector<uint64_t>> timeline_to_latest_timestamp_per_depth_;
};

}  // namespace LinuxTracing

#endif
