// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/CaptureServiceUtils.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace orbit_capture_service {

CaptureRequest WaitForStartCaptureRequestFromClient(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  CaptureRequest request;
  // This call is blocking.
  reader_writer->Read(&request);

  ORBIT_LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
  return request;
}

void WaitForStopCaptureRequestFromClient(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  CaptureRequest request;
  // The client asks for the capture to be stopped by calling WritesDone. At that point, this call
  // to Read will return false. In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");
}

}  // namespace orbit_capture_service
