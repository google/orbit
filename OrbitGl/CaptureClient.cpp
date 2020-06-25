// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient.h"
#include <OrbitBase/Logging.h>

void CaptureClient::Capture() {
  CHECK(reader_writer_ == nullptr);

  grpc::ClientContext context;
  reader_writer_ = capture_service_->Capture(&context);

  CaptureRequest request;
  if (!reader_writer_->Write(request)) {
    reader_writer_->WritesDone();
    FinishCapture();
    return;
  }

  CaptureEvent event;
  while (reader_writer_->Read(&event)) {
  }
  LOG("!reader_writer_->Read(&event)");
  FinishCapture();
}

void CaptureClient::StopCapture() {
  CHECK(reader_writer_ != nullptr);

  if (!reader_writer_->WritesDone()) {
    FinishCapture();
  }
}

void CaptureClient::FinishCapture() {
  if (reader_writer_ == nullptr) {
    return;
  }

  grpc::Status status = reader_writer_->Finish();
  if (!status.ok()) {
    ERROR("Finish(): %s", status.error_message());
  }
  reader_writer_.reset();
}
