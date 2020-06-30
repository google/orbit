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
  tracing_handler.Start(std::move(*request.mutable_capture_options()));

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
    LOG("reader_writer->Read(&request)");
  }
  LOG("!reader_writer->Read(&request)");
  tracing_handler.Stop();

  return grpc::Status::OK;
}
