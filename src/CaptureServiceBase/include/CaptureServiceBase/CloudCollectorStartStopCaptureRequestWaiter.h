// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "CaptureServiceBase/StartStopCaptureRequestWaiter.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_service_base {

// A `StartStopCaptureRequestWaiter` implementation for the cloud collector.
class CloudCollectorStartStopCaptureRequestWaiter : public StartStopCaptureRequestWaiter {
 public:
  // WaitForStartCaptureRequest is blocked until StartCapture is called.
  [[nodiscard]] orbit_grpc_protos::CaptureOptions WaitForStartCaptureRequest() override;
  void StartCapture(orbit_grpc_protos::CaptureOptions capture_options);

  // WaitForStopCaptureRequest is blocked until StopCapture is called.
  void WaitForStopCaptureRequest() override;
  void StopCapture();

 private:
  mutable absl::Mutex start_mutex_;
  orbit_grpc_protos::CaptureOptions capture_options_ ABSL_GUARDED_BY(start_mutex_);
  bool start_requested_ ABSL_GUARDED_BY(start_mutex_) = false;

  mutable absl::Mutex stop_mutex_;
  bool stop_requested_ ABSL_GUARDED_BY(stop_mutex_) = false;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_