// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CloudCollectorStartStopCaptureRequestWaiter.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;

namespace orbit_capture_service_base {

CaptureOptions CloudCollectorStartStopCaptureRequestWaiter::WaitForStartCaptureRequest() {
  // This is blocked until StartCapture is called.
  while (!start_requested_) {
  }

  ORBIT_LOG("Starting capture");
  absl::MutexLock lock(&capture_options_mutex_);
  return capture_options_;
}

void CloudCollectorStartStopCaptureRequestWaiter::StartCapture(CaptureOptions capture_options) {
  {
    absl::MutexLock lock(&capture_options_mutex_);
    capture_options_ = std::move(capture_options);
  }

  ORBIT_LOG("Start capture requested");
  start_requested_ = true;
}

void CloudCollectorStartStopCaptureRequestWaiter::WaitForStopCaptureRequest() {
  // This is blocked until StopCapture is called.
  while (!stop_requested_) {
  }

  ORBIT_LOG("Stopping capture");
}

void CloudCollectorStartStopCaptureRequestWaiter::StopCapture() {
  ORBIT_LOG("Stop capture requested");
  stop_requested_ = true;
}

}  // namespace orbit_capture_service_base