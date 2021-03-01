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

    auto* api_event = capture_event->mutable_api_event();
    api_event->set_num_raw_events(num_events);
    api_event->mutable_raw_data()->Resize(
        num_events * sizeof(orbit_api::ApiEvent) / sizeof(uint64_t), 0);
    void* buffer = api_event->mutable_raw_data()->mutable_data();
    std::memcpy(buffer, intermediate_events, num_events * sizeof(orbit_api::ApiEvent));
    return {capture_event};
  }

  [[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent* TranslateSingleIntermediateEvent(
      orbit_api::ApiEvent&& event, google::protobuf::Arena* arena) override {
    auto* capture_event =
        google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
    auto* api_event = capture_event->mutable_api_event_fixed();
    (void)api_event;
    api_event->set_timestamp_ns(event.timestamp_ns);
    api_event->set_pid(event.pid);
    api_event->set_tid(event.tid);
    api_event->set_type(event.encoded_event.event.type);
    api_event->set_color(event.encoded_event.event.color);
    api_event->set_data(event.encoded_event.event.data);
    char* str = event.encoded_event.event.name;
    uint64_t* str_as_uint64 = reinterpret_cast<uint64_t*>(str);
    api_event->set_d0(str_as_uint64[0]);
    api_event->set_d1(str_as_uint64[1]);
    api_event->set_d2(str_as_uint64[2]);
    api_event->set_d3(str_as_uint64[3]);
    return capture_event;
  }
};

}  // namespace orbit_api

#endif  // ORBIT_API_LOCK_FREE_API_EVENT_PRODUCER
