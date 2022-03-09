// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CloudCollectorStartStopCaptureRequestWaiter.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;

namespace orbit_capture_service_base {

CaptureOptions CloudCollectorStartStopCaptureRequestWaiter::WaitForStartCaptureRequest() {
  absl::MutexLock lock(&start_mutex_);
  start_mutex_.Await(absl::Condition(&start_requested_));
  capture_start_time_ = absl::Now();
  ORBIT_LOG("Starting capture");
  return capture_options_;
}

void CloudCollectorStartStopCaptureRequestWaiter::StartCapture(CaptureOptions capture_options) {
  absl::MutexLock lock(&start_mutex_);
  capture_options_ = std::move(capture_options);
  ORBIT_LOG("Start capture requested");
  start_requested_ = true;
}

CaptureServiceBase::StopCaptureReason
CloudCollectorStartStopCaptureRequestWaiter::WaitForStopCaptureRequest() {
  absl::MutexLock lock(&stop_mutex_);

  if (!max_capture_duration_.has_value()) {
    stop_mutex_.Await(absl::Condition(&stop_requested_));
  } else if (!stop_mutex_.AwaitWithDeadline(absl::Condition(&stop_requested_),
                                            capture_start_time_ + max_capture_duration_.value())) {
    stop_capture_reason_ = CaptureServiceBase::StopCaptureReason::kExceededMaxDurationLimit;
  }

  ORBIT_LOG("Stopping capture");
  return stop_capture_reason_;
}

void CloudCollectorStartStopCaptureRequestWaiter::StopCapture(
    CaptureServiceBase::StopCaptureReason stop_capture_reason) {
  absl::MutexLock lock(&stop_mutex_);
  ORBIT_LOG("Stop capture requested");
  stop_capture_reason_ = std::move(stop_capture_reason);
  stop_requested_ = true;
}

}  // namespace orbit_capture_service_base