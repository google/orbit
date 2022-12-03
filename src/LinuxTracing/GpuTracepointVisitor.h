// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_
#define LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "LinuxTracing/TracerListener.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"

namespace orbit_linux_tracing {

class GpuTracepointVisitor : public PerfEventVisitor {
 public:
  explicit GpuTracepointVisitor(TracerListener* listener) : listener_{listener} {
    ORBIT_CHECK(listener_ != nullptr);
  }

  void Visit(uint64_t event_timestamp, const AmdgpuCsIoctlPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const AmdgpuSchedRunJobPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const DmaFenceSignaledPerfEventData& event_data) override;

 private:
  // Keys are context, seqno, and timeline.
  using Key = std::tuple<uint32_t, uint32_t, std::string>;

  int ComputeDepthForGpuJob(std::string_view timeline, uint64_t start_timestamp,
                            uint64_t end_timestamp);

  void CreateGpuJobAndSendToListenerIfComplete(const Key& key);

  TracerListener* listener_;

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
