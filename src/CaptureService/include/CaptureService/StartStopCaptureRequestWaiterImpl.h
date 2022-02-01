// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_IMPL_H_
#define CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_IMPL_H_

#include <grpcpp/grpcpp.h>

#include "CaptureService/StartStopCaptureRequestWaiter.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service {

class StartStopCaptureRequestWaiterImpl : public StartStopCaptureRequestWaiter {
 public:
  explicit StartStopCaptureRequestWaiterImpl(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  orbit_grpc_protos::CaptureOptions WaitForStartCaptureRequest() override;
  void WaitForStopCaptureRequest() override;

 private:
  grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
      reader_writer_;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_START_STOP_CAPTURE_REQUEST_WAITER_IMPL_H_