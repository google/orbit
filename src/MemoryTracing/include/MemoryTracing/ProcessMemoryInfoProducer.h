// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_PROCESS_MEMORY_INFO_PRODUCER_H_
#define MEMORY_TRACING_PROCESS_MEMORY_INFO_PRODUCER_H_

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoProducer.h"

namespace orbit_memory_tracing {

// This class periodically produces the ProcessMemoryUsage information retrieved from
// /proc/<pid>/stat.
class ProcessMemoryInfoProducer : public MemoryInfoProducer {
 public:
  explicit ProcessMemoryInfoProducer(uint64_t memory_sampling_period_ns, int32_t pid)
      : MemoryInfoProducer(memory_sampling_period_ns), pid_(pid) {}

 private:
  void Run() override;
  void ProduceProcessMemoryUsageAndSendToListener();

  int32_t pid_ = orbit_grpc_protos::kMissingInfo;
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_PROCESS_MEMORY_INFO_PRODUCER_H_