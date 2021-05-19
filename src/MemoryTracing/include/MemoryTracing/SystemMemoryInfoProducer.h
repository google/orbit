// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_SYSTEM_MEMORY_INFO_PRODUCER_H_
#define MEMORY_TRACING_SYSTEM_MEMORY_INFO_PRODUCER_H_

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoProducer.h"

namespace orbit_memory_tracing {

// This class periodically produces the SystemMemoryUsage information retrieved from /proc/meminfo
// and /proc/vmstat.
class SystemMemoryInfoProducer : public MemoryInfoProducer {
 public:
  explicit SystemMemoryInfoProducer(uint64_t memory_sampling_period_ns)
      : MemoryInfoProducer(memory_sampling_period_ns) {}

 private:
  void Run() override;
  void ProduceSystemMemoryUsageAndSendToListener();
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_SYSTEM_MEMORY_INFO_PRODUCER_H_