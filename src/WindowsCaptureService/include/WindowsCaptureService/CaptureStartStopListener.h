// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_SERVICE_CAPTURE_START_STOP_LISTENER_H_
#define WINDOWS_SERVICE_CAPTURE_START_STOP_LISTENER_H_

#include "CaptureEventProcessor/ProducerEventProcessor.h"
#include "capture.pb.h"

namespace windows_service {

// This interface is used to propagate requests received by CaptureServiceImpl to start and stop
// the capture, together with the CaptureOptions and the CaptureEventBuffer where to add the
// generated CaptureEvents.
class CaptureStartStopListener {
 public:
  virtual ~CaptureStartStopListener() = default;

  virtual void OnCaptureStartRequested(
      orbit_grpc_protos::CaptureOptions capture_options,
      capture_event_processor::ProducerEventProcessor* producer_event_processor) = 0;

  // This is to be assumed blocking until the capture stop has been fully processed by the listener.
  virtual void OnCaptureStopRequested() = 0;
};

}  // namespace windows_service

#endif  // WINDOWS_SERVICE_CAPTURE_START_STOP_LISTENER_H_
