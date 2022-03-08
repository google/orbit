// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/GrpcStartStopCaptureRequestWaiter.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace orbit_capture_service_base {

// A `StartStopCaptureRequestWaiter` implementation with `ServerReaderWriter` for the native orbit
// capture services.
class GrpcStartStopCaptureRequestWaiter : public StartStopCaptureRequestWaiter {
 public:
  explicit GrpcStartStopCaptureRequestWaiter(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  [[nodiscard]] CaptureOptions WaitForStartCaptureRequest() override {
    CaptureRequest request;
    // This call is blocking.
    reader_writer_->Read(&request);

    ORBIT_LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
    return request.capture_options();
  }

  [[nodiscard]] CaptureServiceBase::StopCaptureReason WaitForStopCaptureRequest() override {
    CaptureRequest request;
    // The client asks for the capture to be stopped by calling WritesDone. At that point, this
    // call to Read will return false. In the meantime, it blocks if no message is received.
    // Read also unblocks and returns false if the gRPC finishes.
    while (reader_writer_->Read(&request)) {
    }

    ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");
    return CaptureServiceBase::StopCaptureReason::kClientStop;
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;
};

std::shared_ptr<StartStopCaptureRequestWaiter> CreateGrpcStartStopCaptureRequestWaiter(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  return std::make_shared<GrpcStartStopCaptureRequestWaiter>(reader_writer);
}

}  // namespace orbit_capture_service_base