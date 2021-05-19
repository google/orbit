// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICE_MEMORY_INFO_HANDLER_H_
#define SERVICE_MEMORY_INFO_HANDLER_H_

#include "MemoryTracing/MemoryInfoListener.h"
#include "MemoryTracing/SystemMemoryInfoProducer.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor.h"
#include "capture.pb.h"

namespace orbit_service {

// This class controls the start and stop of the MemoryInfoProducer, and also
// receives the SystemMemoryUsage events from the MemoryInfoProducer.
class MemoryInfoHandler : public orbit_memory_tracing::MemoryInfoListener {
 public:
  explicit MemoryInfoHandler(ProducerEventProcessor* producer_event_processor)
      : producer_event_processor_{producer_event_processor} {
    CHECK(producer_event_processor_ != nullptr);
  }

  ~MemoryInfoHandler() override = default;
  MemoryInfoHandler(const MemoryInfoHandler&) = delete;
  MemoryInfoHandler& operator=(const MemoryInfoHandler&) = delete;
  MemoryInfoHandler(MemoryInfoHandler&&) = delete;
  MemoryInfoHandler& operator=(MemoryInfoHandler&&) = delete;

  void Start(orbit_grpc_protos::CaptureOptions capture_options);
  void Stop();

  void OnSystemMemoryUsage(orbit_grpc_protos::SystemMemoryUsage system_memory_usage) override;

 private:
  ProducerEventProcessor* producer_event_processor_;
  std::unique_ptr<orbit_memory_tracing::SystemMemoryInfoProducer> system_memory_info_producer_;
};

}  // namespace orbit_service

#endif  // SERVICE_MEMORY_INFO_HANDLER_H_