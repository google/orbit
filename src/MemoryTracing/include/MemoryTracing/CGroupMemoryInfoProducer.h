// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_CGROUP_MEMORY_INFO_PRODUCER_H_
#define MEMORY_TRACING_CGROUP_MEMORY_INFO_PRODUCER_H_

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoProducer.h"

namespace orbit_memory_tracing {

// This class periodically produces the CGroupMemoryUsage information retrieved from
// /sys/fs/cgroup/memory/<cgroup_name>/memory.stat.
class CGroupMemoryInfoProducer : public MemoryInfoProducer {
 public:
  explicit CGroupMemoryInfoProducer(uint64_t memory_sampling_period_ns, int32_t pid)
      : MemoryInfoProducer(memory_sampling_period_ns), pid_(pid) {}

 private:
  void Run() override;
  void ProduceCGroupMemoryUsageAndSendToListener();

  int32_t pid_ = orbit_grpc_protos::kMissingInfo;
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_CGROUP_MEMORY_INFO_PRODUCER_H_