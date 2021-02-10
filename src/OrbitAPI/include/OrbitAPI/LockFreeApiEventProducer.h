// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
#define ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER

#include "EncodedEvent.h"
#include "OrbitAPI/EncodedEvent.h"
#include "OrbitProducer/LockFreeBufferBulkedCaptureEventProducer.h"
#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "capture.pb.h"
#include "grpcpp/grpcpp.h"

namespace orbit_api {

// This class is used to enqueue orbit_api::Api events from multiple threads and to relay
// them to OrbitService in the form of orbit_grpc_protos::CaptureEvent events.
class LockFreeApiEventProducer
    : public orbit_producer::LockFreeBufferCaptureEventProducer<orbit_api::ApiEvent> {
 public:
  LockFreeApiEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] orbit_grpc_protos::CaptureEvent TranslateIntermediateEvent(
      orbit_api::ApiEvent&& intermediate_event) override {
    orbit_grpc_protos::ApiEvent api_event;
    api_event.set_num_raw_events(1);
    api_event.set_raw_data(&intermediate_event, sizeof(orbit_api::ApiEvent));
    orbit_grpc_protos::CaptureEvent capture_event;
    *capture_event.mutable_api_event() = std::move(api_event);
    return capture_event;
  }
};

// This class is used to enqueue orbit_api::Api events from multiple threads and to relay
// them to OrbitService in the form of orbit_grpc_protos::CaptureEvent events.
class LockFreeApiEventBulkProducer
    : public orbit_producer::LockFreeBufferBulkedCaptureEventProducer<orbit_api::ApiEvent> {
 public:
  LockFreeApiEventBulkProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventBulkProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] orbit_grpc_protos::CaptureEvent TranslateIntermediateEvents(
      orbit_api::ApiEvent* intermediate_events, size_t size) override {
    orbit_grpc_protos::ApiEvent api_event;
    api_event.set_num_raw_events(size);
    api_event.set_raw_data(intermediate_events, size * sizeof(orbit_api::ApiEvent));

    orbit_grpc_protos::CaptureEvent capture_event;
    *capture_event.mutable_api_event() = std::move(api_event);
    return capture_event;
  }
};

}  // namespace orbit_api

#endif  // ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
