// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_
#define LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "TracingInterface/TracerListener.h"

namespace orbit_linux_tracing {

class GpuTracepointVisitor : public PerfEventVisitor {
 public:
  explicit GpuTracepointVisitor(orbit_tracing_interface::TracerListener* listener)
      : listener_{listener} {
    CHECK(listener_ != nullptr);
  }

  void Visit(AmdgpuCsIoctlPerfEvent* event) override;
  void Visit(AmdgpuSchedRunJobPerfEvent* event) override;
  void Visit(DmaFenceSignaledPerfEvent* event) override;

 private:
  // Keys are context, seqno, and timeline.
  using Key = std::tuple<uint32_t, uint32_t, std::string>;

  int ComputeDepthForGpuJob(const std::string& timeline, uint64_t start_timestamp,
                            uint64_t end_timestamp);

  void CreateGpuJobAndSendToListenerIfComplete(const Key& key);

  orbit_tracing_interface::TracerListener* listener_;

  struct AmdgpuCsIoctlEvent {
    pid_t pid;
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

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_
