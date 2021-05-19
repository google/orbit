// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryInfoHandler.h"

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

namespace orbit_service {

void MemoryInfoHandler::Start(orbit_grpc_protos::CaptureOptions capture_options) {
  CHECK(system_memory_info_producer_ == nullptr);

  if (!capture_options.collect_memory_info()) return;

  system_memory_info_producer_ = std::make_unique<orbit_memory_tracing::SystemMemoryInfoProducer>(
      capture_options.memory_sampling_period_ns());
  system_memory_info_producer_->SetListener(this);
  system_memory_info_producer_->Start();
}

void MemoryInfoHandler::Stop() {
  if (system_memory_info_producer_ == nullptr) return;
  system_memory_info_producer_->Stop();
  system_memory_info_producer_.reset();
}

void MemoryInfoHandler::OnSystemMemoryUsage(
    orbit_grpc_protos::SystemMemoryUsage system_memory_usage) {
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_system_memory_usage() = std::move(system_memory_usage);
  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kMemoryInfoProducerId,
                                          std::move(event));
}

}  // namespace orbit_service
