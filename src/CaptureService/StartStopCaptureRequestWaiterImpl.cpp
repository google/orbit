// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/StartStopCaptureRequestWaiterImpl.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;

namespace orbit_capture_service {

orbit_grpc_protos::CaptureOptions StartStopCaptureRequestWaiterImpl::WaitForStartCaptureRequest() {
  orbit_grpc_protos::CaptureRequest request;
  // This call is blocking.
  reader_writer_->Read(&request);

  ORBIT_LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
  return request.capture_options();
}

void StartStopCaptureRequestWaiterImpl::WaitForStopCaptureRequest() {
  orbit_grpc_protos::CaptureRequest request;
  // The client asks for the capture to be stopped by calling WritesDone. At that point, this
  // call to Read will return false. In the meantime, it blocks if no message is received.
  // Read also unblocks and returns false if the gRPC finishes.
  while (reader_writer_->Read(&request)) {
  }
  ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");
}

}  // namespace orbit_capture_service