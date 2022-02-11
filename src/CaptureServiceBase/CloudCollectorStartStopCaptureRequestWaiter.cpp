// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CloudCollectorStartStopCaptureRequestWaiter.h"

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;

namespace orbit_capture_service_base {

// A `StartStopCaptureRequestWaiter` implementation for the cloud collector.
class CloudCollectorStartStopCaptureRequestWaiter : public StartStopCaptureRequestWaiter {
 public:
  [[nodiscard]] CaptureOptions WaitForStartCaptureRequest() override {
    // This is blocked until StartCapture is called.
    while (!start_requested_) {
    }

    ORBIT_LOG("Starting capture");
    absl::MutexLock lock(&capture_options_mutex_);
    return capture_options_;
  }

  void StartCapture(CaptureOptions capture_options) {
    {
      absl::MutexLock lock(&capture_options_mutex_);
      capture_options_ = std::move(capture_options);
    }

    ORBIT_LOG("Start capture requested");
    start_requested_ = true;
  }

  void WaitForStopCaptureRequest() override {
    // This is blocked until StopCapture is called.
    while (!stop_requested_) {
    }

    ORBIT_LOG("Stopping capture");
  }

  void StopCapture() {
    ORBIT_LOG("Stop capture requested");
    stop_requested_ = true;
  }

 private:
  mutable absl::Mutex capture_options_mutex_;
  CaptureOptions capture_options_ ABSL_GUARDED_BY(capture_options_mutex_);

  std::atomic<bool> start_requested_ = false;
  std::atomic<bool> stop_requested_ = false;
};

std::shared_ptr<StartStopCaptureRequestWaiter> CreateCloudCollectorStartStopCaptureRequestWaiter() {
  return std::make_shared<CloudCollectorStartStopCaptureRequestWaiter>();
}

}  // namespace orbit_capture_service_base