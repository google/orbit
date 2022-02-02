// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_
#define LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_

#include <absl/container/flat_hash_set.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <thread>

#include "CaptureService/CaptureServiceBase.h"
#include "CaptureService/StartStopCaptureRequestWaiter.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"

namespace orbit_linux_capture_service {

// Linux implementation of the grpc capture service.
class LinuxCaptureService final : public orbit_capture_service::CaptureServiceBase,
                                  public orbit_grpc_protos::CaptureService::Service {
 public:
  LinuxCaptureService() {
    instrumentation_manager_ = orbit_user_space_instrumentation::InstrumentationManager::Create();
  }

  ~LinuxCaptureService() override {
    if (wait_for_stop_capture_request_thread_.joinable()) {
      wait_for_stop_capture_request_thread_.join();
    }
  }

  grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) override;

 private:
  std::unique_ptr<orbit_user_space_instrumentation::InstrumentationManager>
      instrumentation_manager_;

  // This method returns when the first of the following happens:
  // - The client calls WritesDone on reader_writer (similar to
  //   CaptureService::WaitForStopCaptureRequestFromClient);
  // - MemoryWatchdog calls OnThresholdExceeded.
  [[nodiscard]] StopCaptureReason WaitForStopCaptureRequestOrMemoryThresholdExceeded(
      const std::shared_ptr<orbit_capture_service::StartStopCaptureRequestWaiter>&
          start_stop_capture_request_waiter);
  std::thread wait_for_stop_capture_request_thread_;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_
