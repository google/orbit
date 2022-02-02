// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service {

// This is mimicking the behavior of `ServerReaderWriter`. Native orbit capture services can still
// implement it with `ServerReaderWriter` and the cloud collector can implement it in a gRPC-free
// way.
class StartStopCaptureRequestWaiter {
 public:
  virtual ~StartStopCaptureRequestWaiter() = default;
  virtual orbit_grpc_protos::CaptureOptions WaitForStartCaptureRequest() = 0;
  virtual void WaitForStopCaptureRequest() = 0;
};

// Create a `GrpcStartStopCaptureRequestWaiter` with `ServerReaderWriter` for the native orbit
// capture services.
std::shared_ptr<StartStopCaptureRequestWaiter> CreateGrpcStartStopCaptureRequestWaiter(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer);

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_H_