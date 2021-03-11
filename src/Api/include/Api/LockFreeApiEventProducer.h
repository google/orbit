// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef API_LOCK_FREE_API_EVENT_PRODUCER_H_
#define API_LOCK_FREE_API_EVENT_PRODUCER_H_

#include "Api/EncodedEvent.h"
#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "ProducerSideChannel/ProducerSideChannel.h"

namespace orbit_api {

// This class is used to enqueue orbit_api::ApiEvent events from multiple threads and relay them to
// OrbitService in the form of orbit_grpc_protos::ApiEvent events.
class LockFreeApiEventProducer
    : public orbit_producer::LockFreeBufferCaptureEventProducer<orbit_api::ApiEvent> {
 public:
  LockFreeApiEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] virtual orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      orbit_api::ApiEvent&& raw_api_event, google::protobuf::Arena* arena) override {
    auto* capture_event =
        google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
    auto* api_event = capture_event->mutable_api_event();
    api_event->set_timestamp_ns(raw_api_event.timestamp_ns);
    api_event->set_pid(raw_api_event.pid);
    api_event->set_tid(raw_api_event.tid);
    api_event->set_r0(raw_api_event.encoded_event.args[0]);
    api_event->set_r1(raw_api_event.encoded_event.args[1]);
    api_event->set_r2(raw_api_event.encoded_event.args[2]);
    api_event->set_r3(raw_api_event.encoded_event.args[3]);
    api_event->set_r4(raw_api_event.encoded_event.args[4]);
    api_event->set_r5(raw_api_event.encoded_event.args[5]);
    return capture_event;
  }
};

}  // namespace orbit_api

#endif  // API_LOCK_FREE_API_EVENT_PRODUCER_H_
