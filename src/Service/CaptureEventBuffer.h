// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_CAPTURE_EVENT_BUFFER_H_
#define ORBIT_SERVICE_CAPTURE_EVENT_BUFFER_H_

#include "capture.pb.h"

namespace orbit_service {

// Interface used to buffer CaptureEvents so that multiple CaptureEvents
// can be processed at the same time (e.g., grouped into fewer bigger CaptureResponses).
// AddEvent is to be assumed thread safe.
class CaptureEventBuffer {
 public:
  virtual ~CaptureEventBuffer() = default;
  virtual void AddEvent(orbit_grpc_protos::ClientCaptureEvent&& event) = 0;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_CAPTURE_EVENT_BUFFER_H_
