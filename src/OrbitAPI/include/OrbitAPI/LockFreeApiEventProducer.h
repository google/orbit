// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
#define ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER

#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "capture.pb.h"
#include "grpcpp/grpcpp.h"

namespace orbit_api {

// This class is used to enqueue orbit_api::EncodedEvent events from multiple threads and relaying
// them to OrbitService in the form of orbit_grpc_protos::CaptureEvent events.
class LockFreeApiEventProducer
    : public orbit_producer::LockFreeBufferCaptureEventProducer<orbit_grpc_protos::CaptureEvent> {
 public:
  LockFreeApiEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventProducer() { ShutdownAndWait(); }

  orbit_grpc_protos::CaptureEvent TranslateIntermediateEvent(
      orbit_grpc_protos::CaptureEvent&& intermediate_event) override {
    // TODO: if the intermediate type is a orbit_grpc_protos::CaptureEvent, we shouldn't need to
    //       translate the event. There is avoidable data copy here.
    return std::move(intermediate_event);
  }
};

}  // namespace orbit_api

#endif  // ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
