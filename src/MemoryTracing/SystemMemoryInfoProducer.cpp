// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracing/SystemMemoryInfoProducer.h"

#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>

#include "MemoryTracingUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::SystemMemoryUsage;

void SystemMemoryInfoProducer::Run() {
  orbit_base::SetCurrentThreadName("SysMemPr::Run");

  CHECK(listener_ != nullptr);
  ProduceSystemMemoryUsageAndSendToListener();
}

void SystemMemoryInfoProducer::ProduceSystemMemoryUsageAndSendToListener() {
  absl::Time scheduled_time = absl::Now();
  absl::MutexLock lock(&exit_requested_mutex_);
  while (!exit_requested_) {
    ErrorMessageOr<SystemMemoryUsage> system_memory_usage = GetSystemMemoryUsage();
    if (system_memory_usage.has_value()) {
      listener_->OnSystemMemoryUsage(system_memory_usage.value());
    }
    scheduled_time += absl::Nanoseconds(sampling_period_ns_);
    exit_requested_mutex_.AwaitWithDeadline(absl::Condition(&exit_requested_), scheduled_time);
  }
}

}  // namespace orbit_memory_tracing