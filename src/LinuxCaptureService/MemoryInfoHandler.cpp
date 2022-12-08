// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryInfoHandler.h"

#include <sys/types.h>

#include <utility>

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryTracingUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_linux_capture_service {

void MemoryInfoHandler::Start(const orbit_grpc_protos::CaptureOptions& capture_options) {
  if (!capture_options.collect_memory_info()) return;

  SetSamplingStartTimestampNs(orbit_base::CaptureTimestampNs());
  SetSamplingPeriodNs(capture_options.memory_sampling_period_ns());
  SetEnableCGroupMemory(
      orbit_memory_tracing::GetCGroupMemoryUsage(capture_options.pid()).has_value());
  SetEnableProcessMemory(true);

  const pid_t pid = orbit_base::ToNativeProcessId(capture_options.pid());

  ORBIT_CHECK(system_memory_info_producer_ == nullptr);
  system_memory_info_producer_ = orbit_memory_tracing::CreateSystemMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), pid);
  system_memory_info_producer_->Start();

  ORBIT_CHECK(cgroup_memory_info_producer_ == nullptr);
  cgroup_memory_info_producer_ = orbit_memory_tracing::CreateCGroupMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), pid);
  cgroup_memory_info_producer_->Start();

  ORBIT_CHECK(process_memory_info_producer_ == nullptr);
  process_memory_info_producer_ = orbit_memory_tracing::CreateProcessMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), pid);
  process_memory_info_producer_->Start();
}

void MemoryInfoHandler::Stop() {
  if (system_memory_info_producer_ != nullptr) {
    system_memory_info_producer_->Stop();
    system_memory_info_producer_.reset();
  }

  if (cgroup_memory_info_producer_ != nullptr) {
    cgroup_memory_info_producer_->Stop();
    cgroup_memory_info_producer_.reset();
  }

  if (process_memory_info_producer_ != nullptr) {
    process_memory_info_producer_->Stop();
    process_memory_info_producer_.reset();
  }
}

void MemoryInfoHandler::OnMemoryUsageEvent(orbit_grpc_protos::MemoryUsageEvent memory_usage_event) {
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_memory_usage_event() = std::move(memory_usage_event);
  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kMemoryInfoProducerId,
                                          std::move(event));
}

}  // namespace orbit_linux_capture_service
