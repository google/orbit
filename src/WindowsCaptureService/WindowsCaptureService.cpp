// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <stdint.h>

#include "OrbitBase/ThreadUtils.h"
#include "TracingHandler.h"
#include "capture.pb.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

grpc::Status WindowsCaptureService::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("WinCS::Capture");

  ErrorMessageOr<void> result = InitializeCapture(reader_writer);
  if (result.has_error()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, result.error().message());
  }

  TracingHandler tracing_handler{producer_event_processor_.get()};
  CaptureRequest request;

  WaitForStartCaptureRequestFromClient(reader_writer, request);

  StartEventProcessing(request.capture_options());
  tracing_handler.Start(request.capture_options());

  WaitForEndCaptureRequestFromClient(reader_writer, request);

  tracing_handler.Stop();
  LOG("Windows TracingHandler stopped: etw tracing is done");
  FinalizeEventProcessing();

  TerminateCapture();

  return grpc::Status::OK;
}

}  // namespace orbit_windows_capture_service
