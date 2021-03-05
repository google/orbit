// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_
#define LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "LinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"

namespace orbit_linux_tracing {

class GpuTracepointVisitor : public PerfEventVisitor {
 public:
  void SetListener(TracerListener* listener);

  void visit(AmdgpuCsIoctlPerfEvent* event) override;
  void visit(AmdgpuSchedRunJobPerfEvent* event) override;
  void visit(DmaFenceSignaledPerfEvent* event) override;

 private:
  // Keys are context, seqno, and timeline.
  using Key = std::tuple<uint32_t, uint32_t, std::string>;

  int ComputeDepthForGpuJob(const std::string& timeline, uint64_t start_timestamp,
                            uint64_t end_timestamp);

  void CreateGpuJobAndSendToListenerIfComplete(const Key& key);

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

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_
