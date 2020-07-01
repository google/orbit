// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceImpl.h"

#include <OrbitBase/Logging.h>

#include "LinuxTracingGrpcHandler.h"

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureEvent, CaptureRequest>* reader_writer) {
  LinuxTracingGrpcHandler tracing_handler{reader_writer};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
  tracing_handler.Start(std::move(*request.mutable_capture_options()));

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");
  tracing_handler.Stop();

  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  return grpc::Status::OK;
}
