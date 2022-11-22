// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_BASE_H_
#define LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_BASE_H_

#include <absl/container/flat_hash_set.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <thread>

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "CaptureServiceBase/StopCaptureRequestWaiter.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"

namespace orbit_linux_capture_service {

// This class is gRPC-free and provides common functionality that is shared by the native Orbit
// Linux capture service and the cloud collector.
class LinuxCaptureServiceBase : public orbit_capture_service_base::CaptureServiceBase {
 public:
  LinuxCaptureServiceBase() {
    instrumentation_manager_ = orbit_user_space_instrumentation::InstrumentationManager::Create();
  }

  ~LinuxCaptureServiceBase() {
    if (wait_for_stop_capture_request_thread_.joinable()) {
      wait_for_stop_capture_request_thread_.join();
    }
  }

  // Note that stop_capture_request_waiter needs to be a shared_ptr here as it might outlive
  // this method. See wait_for_stop_capture_request_thread_ in
  // WaitForStopCaptureRequestOrMemoryThresholdExceeded.
  void DoCapture(const orbit_grpc_protos::CaptureOptions& capture_options,
                 const std::shared_ptr<orbit_capture_service_base::StopCaptureRequestWaiter>&
                     stop_capture_request_waiter);

 private:
  std::unique_ptr<orbit_user_space_instrumentation::InstrumentationManager>
      instrumentation_manager_;

  // This method returns when the first of the following happens:
  // - stop_capture_request_waiter->WaitForStopCaptureRequest returns. For the native Orbit
  //   capture service with a GrpcStartStopCaptureRequestWaiter, this happens when the client calls
  //   WritesDone on reader_writer; For the cloud collector capture service with a
  //   CloudCollectorStartStopCaptureRequestWaiter, this happends when
  //   CloudCollectorStartStopCaptureRequestWaiter::StopCapture is called.
  // - The resident set size of the current process exceeds the threshold (i.e., total physical
  //   memory / 2).
  [[nodiscard]] StopCaptureReason WaitForStopCaptureRequestOrMemoryThresholdExceeded(
      const std::shared_ptr<orbit_capture_service_base::StopCaptureRequestWaiter>&
          stop_capture_request_waiter);
  std::thread wait_for_stop_capture_request_thread_;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_BASE_H_
