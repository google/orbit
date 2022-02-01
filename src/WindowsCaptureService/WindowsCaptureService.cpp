// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include "CaptureService/CaptureServiceUtils.h"
#include "CaptureService/ClientCaptureEventCollectorBuilderImpl.h"
#include "CaptureService/StartStopCaptureRequestWaiterImpl.h"
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

  orbit_capture_service::ClientCaptureEventCollectorBuilderImpl
      client_capture_event_collector_builder{reader_writer};

  // shared_ptr because it might outlive this method. See wait_for_stop_capture_request_thread_ in
  // WaitForStopCaptureRequestOrMemoryThresholdExceeded.
  auto start_stop_capture_request_waiter =
      std::make_shared<orbit_capture_service::StartStopCaptureRequestWaiterImpl>(reader_writer);

  if (CaptureServiceBase::CaptureInitializationResult result =
          InitializeCapture(&client_capture_event_collector_builder);
      result == CaptureServiceBase::CaptureInitializationResult::kAlreadyInProgress) {
    return {grpc::StatusCode::ALREADY_EXISTS,
            "Cannot start capture because another capture is already in progress"};
  }

  const CaptureOptions& capture_options =
      start_stop_capture_request_waiter->WaitForStartCaptureRequest();

  StartEventProcessing(capture_options);
  TracingHandler tracing_handler{producer_event_processor_.get()};
  tracing_handler.Start(capture_options);
  start_stop_capture_request_waiter->WaitForStopCaptureRequest();
  tracing_handler.Stop();
  FinalizeEventProcessing(StopCaptureReason::kClientStop);

  TerminateCapture();

  return grpc::Status::OK;
}

}  // namespace orbit_windows_capture_service
