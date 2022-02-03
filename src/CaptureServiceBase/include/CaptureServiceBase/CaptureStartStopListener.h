// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CAPTURE_START_STOP_LISTENER_H_
#define CAPTURE_SERVICE_BASE_CAPTURE_START_STOP_LISTENER_H_

#include "GrpcProtos/capture.pb.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service_base {

// This interface is used to propagate requests received by CaptureServiceImpl to start and stop
// the capture, together with the CaptureOptions and the CaptureEventBuffer where to add the
// generated CaptureEvents.
class CaptureStartStopListener {
 public:
  virtual ~CaptureStartStopListener() = default;

  virtual void OnCaptureStartRequested(
      orbit_grpc_protos::CaptureOptions capture_options,
      orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor) = 0;

  // This is to be assumed blocking until the capture stop has been fully processed by the listener.
  virtual void OnCaptureStopRequested() = 0;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CAPTURE_START_STOP_LISTENER_H_
