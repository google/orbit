// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_CAPTURE_RESPONSE_LISTENER_H_
#define ORBIT_SERVICE_CAPTURE_RESPONSE_LISTENER_H_

#include "capture.pb.h"

// This interface is used by LinuxTracingHandler
// to process a buffered vector of generated events.
class CaptureResponseListener {
 public:
  virtual ~CaptureResponseListener() = default;
  virtual void ProcessEvents(std::vector<orbit_grpc_protos::CaptureEvent>&& events) = 0;
};

#endif  // ORBIT_SERVICE_CAPTURE_RESPONSE_LISTENER_H_
