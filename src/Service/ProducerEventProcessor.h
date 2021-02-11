// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICE_PRODUCER_EVENT_PROCESSOR_H_
#define SERVICE_PRODUCER_EVENT_PROCESSOR_H_

#include <stdint.h>

#include "CaptureEventBuffer.h"
#include "capture.pb.h"

namespace orbit_service {

// The implementation of this interface is responsible for processing
// ProducerCaptureEvents from multiple producers.
// It converts them to ClientCaptureEvents and sends to CaptureEventBuffer.
class ProducerEventProcessor {
 public:
  ProducerEventProcessor() = default;
  virtual ~ProducerEventProcessor() = default;

  virtual void ProcessEvent(uint64_t producer_id,
                            orbit_grpc_protos::ProducerCaptureEvent event) = 0;

  static std::unique_ptr<ProducerEventProcessor> Create(CaptureEventBuffer* capture_event_buffer);
};

}  // namespace orbit_service

#endif  // SERVICE_PRODUCER_EVENT_PROCESSOR_H_
