// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_BASE_STOP_CAPTURE_REQUEST_WAITER_H_

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_service_base {

// This is mimicking the behavior of `ServerReaderWriter`. Native orbit capture services can still
// implement it with `ServerReaderWriter` and the cloud collector can implement it in a gRPC-free
// way.
class StopCaptureRequestWaiter {
 public:
  virtual ~StopCaptureRequestWaiter() = default;
  [[nodiscard]] virtual CaptureServiceBase::StopCaptureReason WaitForStopCaptureRequest() = 0;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_STOP_CAPTURE_REQUEST_WAITER_H_