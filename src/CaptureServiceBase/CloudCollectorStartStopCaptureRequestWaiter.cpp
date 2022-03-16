// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CloudCollectorStartStopCaptureRequestWaiter.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;

namespace orbit_capture_service_base {

ErrorMessageOr<CaptureOptions>
CloudCollectorStartStopCaptureRequestWaiter::WaitForStartCaptureRequest() {
  absl::MutexLock lock(&mutex_);
  mutex_.Await(absl::Condition(
      +[](CloudCollectorStartStopCaptureRequestWaiter* waiter) {
        waiter->mutex_.AssertReaderHeld();
        return waiter->start_requested_ || waiter->stop_requested_;
      },
      this));

  if (!start_requested_) return ErrorMessage("Stop capture requested before start");

  ORBIT_LOG("Starting capture");
  return capture_options_;
}

void CloudCollectorStartStopCaptureRequestWaiter::StartCapture(CaptureOptions capture_options) {
  absl::MutexLock lock(&mutex_);
  capture_options_ = std::move(capture_options);
  ORBIT_LOG("Start capture requested");
  start_requested_ = true;
}

CaptureServiceBase::StopCaptureReason
CloudCollectorStartStopCaptureRequestWaiter::WaitForStopCaptureRequest() {
  absl::MutexLock lock(&mutex_);

  if (!max_capture_duration_.has_value()) {
    mutex_.Await(absl::Condition(&stop_requested_));
  } else if (!mutex_.AwaitWithTimeout(absl::Condition(&stop_requested_),
                                      max_capture_duration_.value())) {
    stop_capture_reason_ = CaptureServiceBase::StopCaptureReason::kExceededMaxDurationLimit;
  }

  ORBIT_LOG("Stopping capture");
  return stop_capture_reason_;
}

void CloudCollectorStartStopCaptureRequestWaiter::StopCapture(
    CaptureServiceBase::StopCaptureReason stop_capture_reason) {
  absl::MutexLock lock(&mutex_);
  ORBIT_LOG("Stop capture requested");
  stop_capture_reason_ = std::move(stop_capture_reason);
  stop_requested_ = true;
}

CaptureServiceBase::StopCaptureReason
CloudCollectorStartStopCaptureRequestWaiter::GetStopCaptureReason() const {
  absl::MutexLock lock(&mutex_);
  return stop_capture_reason_;
}

}  // namespace orbit_capture_service_base