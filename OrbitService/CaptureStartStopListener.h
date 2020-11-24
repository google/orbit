// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_CAPTURE_START_STOP_LISTENER_H_
#define ORBIT_SERVICE_CAPTURE_START_STOP_LISTENER_H_

#include "CaptureEventBuffer.h"

namespace orbit_service {

// This interface is used to propagate requests received by CaptureServiceImpl to start and stop
// the capture, together with the CaptureEventBuffer where to add the generated CaptureEvents.
class CaptureStartStopListener {
 public:
  virtual ~CaptureStartStopListener() = default;

  virtual void OnCaptureStartRequested(CaptureEventBuffer* capture_event_buffer) = 0;

  // This is to be assumed blocking until the capture stop has been fully processed by the listener.
  virtual void OnCaptureStopRequested() = 0;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_CAPTURE_START_STOP_LISTENER_H_
