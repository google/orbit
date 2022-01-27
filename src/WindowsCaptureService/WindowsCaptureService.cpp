// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <stdint.h>

#include "CaptureService/CaptureServiceUtils.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/ThreadUtils.h"
#include "TracingHandler.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

grpc::Status WindowsCaptureService::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("WinCS::Capture");

  grpc::Status result = InitializeCapture(reader_writer);
  if (!result.ok()) {
    return result;
  }

  CaptureRequest request = WaitForStartCaptureRequestFromClient(reader_writer);

  orbit_capture_service::StartEventProcessing(request.capture_options(), meta_data_);
  TracingHandler tracing_handler{meta_data_.producer_event_processor.get()};
  tracing_handler.Start(request.capture_options());
  WaitForStopCaptureRequestFromClient(reader_writer);
  tracing_handler.Stop();
  orbit_capture_service::FinalizeEventProcessing(
      orbit_capture_service::StopCaptureReason::kClientStop, meta_data_);

  TerminateCapture();

  return grpc::Status::OK;
}

}  // namespace orbit_windows_capture_service
