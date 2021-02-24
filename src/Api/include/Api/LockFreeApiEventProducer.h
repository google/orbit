// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
#define ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER

#include "Api/EncodedEvent.h"
#include "EncodedEvent.h"
#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "capture.pb.h"
#include "grpcpp/grpcpp.h"

namespace orbit_api {

// This class is used to enqueue orbit_api::Api events from multiple threads and relay them to
// OrbitService in the form of orbit_grpc_protos::ProducerCaptureEventCaptureEvent events.
class LockFreeApiEventProducer
    : public orbit_producer::LockFreeBufferCaptureEventProducer<orbit_api::ApiEvent> {
 public:
  LockFreeApiEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] std::vector<orbit_grpc_protos::ProducerCaptureEvent*> TranslateIntermediateEvents(
      orbit_api::ApiEvent* intermediate_events, size_t num_events,
      google::protobuf::Arena* arena) override {
    auto* capture_event =
        google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
    auto* api_event = google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ApiEvent>(arena);

    api_event->set_num_raw_events(num_events);
    api_event->set_raw_data(intermediate_events, num_events * sizeof(orbit_api::ApiEvent));
    *capture_event->mutable_api_event() = std::move(*api_event);
    return {capture_event};
  }
};

}  // namespace orbit_api

#endif  // ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
