// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_EVENT_PROCESSOR_EVENT_PROCESSOR_H_
#define CAPTURE_EVENT_PROCESSOR_EVENT_PROCESSOR_H_

#include <stdint.h>

#include <memory>

#include "ClientCaptureEventCollector.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_producer_event_processor {

// The implementation of this interface is responsible for processing
// ProducerCaptureEvents from multiple producers.
// It converts them to ClientCaptureEvents and sends to CaptureEventBuffer.
class ProducerEventProcessor {
 public:
  ProducerEventProcessor() = default;
  virtual ~ProducerEventProcessor() = default;

  virtual void ProcessEvent(uint64_t producer_id,
                            orbit_grpc_protos::ProducerCaptureEvent&& event) = 0;

  static std::unique_ptr<ProducerEventProcessor> Create(
      ClientCaptureEventCollector* client_capture_event_collector);
};

}  // namespace orbit_producer_event_processor

#endif  // CAPTURE_EVENT_PROCESSOR_EVENT_PROCESSOR_H_
