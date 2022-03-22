// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>

#include "CaptureServiceBase/StopCaptureRequestWaiter.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_service_base {

// A `StartStopCaptureRequestWaiter` implementation for the cloud collector.
class CloudCollectorStartStopCaptureRequestWaiter : public StopCaptureRequestWaiter {
 public:
  explicit CloudCollectorStartStopCaptureRequestWaiter(
      std::optional<absl::Duration> max_capture_duration = std::nullopt)
      : max_capture_duration_(max_capture_duration) {}

  // WaitForStartCaptureRequest is blocked until StartCapture or StopCapture is called. For the
  // latter case, it will return an error message.
  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::CaptureOptions> WaitForStartCaptureRequest();
  void StartCapture(orbit_grpc_protos::CaptureOptions capture_options);

  // WaitForStopCaptureRequest is blocked until StopCapture is called.
  [[nodiscard]] CaptureServiceBase::StopCaptureReason WaitForStopCaptureRequest() override;
  void StopCapture(CaptureServiceBase::StopCaptureReason stop_capture_reason);

  [[nodiscard]] CaptureServiceBase::StopCaptureReason GetStopCaptureReason() const;

 private:
  mutable absl::Mutex mutex_;
  orbit_grpc_protos::CaptureOptions capture_options_ ABSL_GUARDED_BY(mutex_);
  bool start_requested_ ABSL_GUARDED_BY(mutex_) = false;

  CaptureServiceBase::StopCaptureReason stop_capture_reason_ ABSL_GUARDED_BY(mutex_);
  bool stop_requested_ ABSL_GUARDED_BY(mutex_) = false;

  std::optional<absl::Duration> max_capture_duration_;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_