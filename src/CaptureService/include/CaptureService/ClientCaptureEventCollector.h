// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_H_
#define CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_H_

#include "capture.pb.h"

namespace orbit_capture_service {

// Interface used to receive ClientCaptureEvents from a ProducerEventProcessor.
// AddEvent is to be assumed thread safe.
class ClientCaptureEventCollector {
 public:
  virtual ~ClientCaptureEventCollector() = default;
  virtual void AddEvent(orbit_grpc_protos::ClientCaptureEvent&& event) = 0;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_H_
