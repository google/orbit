// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef API_LOCK_FREE_API_EVENT_PRODUCER_H_
#define API_LOCK_FREE_API_EVENT_PRODUCER_H_

#include <google/protobuf/arena.h>

#include <utility>
#include <variant>

#include "ApiUtils/Event.h"
#include "CaptureEventProducer/LockFreeBufferCaptureEventProducer.h"
#include "GrpcProtos/capture.pb.h"
#include "ProducerSideChannel/ProducerSideChannel.h"

namespace orbit_api {

// This class is used to enqueue orbit_api::ApiEvent events from multiple threads and relay them to
// OrbitService in the form of orbit_grpc_protos::ApiEvent events.
class LockFreeApiEventProducer
    : public orbit_capture_event_producer::LockFreeBufferCaptureEventProducer<ApiEventVariant> {
 public:
  LockFreeApiEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeApiEventProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      ApiEventVariant&& raw_api_event, google::protobuf::Arena* arena) override;
};

}  // namespace orbit_api

#endif  // API_LOCK_FREE_API_EVENT_PRODUCER_H_
