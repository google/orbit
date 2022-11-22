// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_GRPC_START_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_BASE_GRPC_START_STOP_CAPTURE_REQUEST_WAITER_H_

#include <grpcpp/grpcpp.h>

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "CaptureServiceBase/StopCaptureRequestWaiter.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service_base {

// A `StartStopCaptureRequestWaiter` with `ServerReaderWriter` for the native orbit capture
// services.
class GrpcStartStopCaptureRequestWaiter : public StopCaptureRequestWaiter {
 public:
  explicit GrpcStartStopCaptureRequestWaiter(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  [[nodiscard]] orbit_grpc_protos::CaptureOptions WaitForStartCaptureRequest();
  [[nodiscard]] CaptureServiceBase::StopCaptureReason WaitForStopCaptureRequest() override;

 private:
  grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
      reader_writer_;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_GRPC_START_STOP_CAPTURE_REQUEST_WAITER_H_