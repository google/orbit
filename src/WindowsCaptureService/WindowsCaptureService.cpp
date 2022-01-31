// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include "CaptureService/CaptureServiceUtils.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "TracingHandler.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_producer_event_processor::GrpcClientCaptureEventCollector;

grpc::Status WindowsCaptureService::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("WinCS::Capture");

  if (ErrorMessageOr<void> result =
          InitializeCapture(std::make_unique<GrpcClientCaptureEventCollector>(reader_writer));
      result.has_error()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, result.error().message());
  }

  CaptureRequest request =
      orbit_capture_service::WaitForStartCaptureRequestFromClient(reader_writer);

  StartEventProcessing(request.capture_options());
  TracingHandler tracing_handler{producer_event_processor_.get()};
  tracing_handler.Start(request.capture_options());
  orbit_capture_service::WaitForStopCaptureRequestFromClient(reader_writer);
  tracing_handler.Stop();
  FinalizeEventProcessing(StopCaptureReason::kClientStop);

  TerminateCapture();

  return grpc::Status::OK;
}

}  // namespace orbit_windows_capture_service
