// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitBase/Logging.h>

#include "CaptureServiceImpl.h"

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureEvent, CaptureRequest>* reader_writer) {
  CaptureRequest request;
  while (reader_writer->Read(&request)) {
    LOG("reader_writer->Read(&request)");
  }
  LOG("!reader_writer->Read(&request)");

  return grpc::Status::OK;
}
