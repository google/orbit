// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxCaptureService/LinuxCaptureService.h"

#include "CaptureServiceBase/GrpcStartStopCaptureRequestWaiter.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

namespace orbit_linux_capture_service {

grpc::Status LinuxCaptureService::Capture(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer) {
  orbit_base::SetCurrentThreadName("CSImpl::Capture");

  orbit_producer_event_processor::GrpcClientCaptureEventCollector
      grpc_client_capture_event_collector{reader_writer};
  CaptureServiceBase::CaptureInitializationResult initialization_result =
      InitializeCapture(&grpc_client_capture_event_collector);
  switch (initialization_result) {
    case CaptureInitializationResult::kSuccess:
      break;
    case CaptureInitializationResult::kAlreadyInProgress:
      return {grpc::StatusCode::ALREADY_EXISTS,
              "Cannot start capture because another capture is already in progress"};
  }

  std::shared_ptr<orbit_capture_service_base::StartStopCaptureRequestWaiter>
      start_stop_capture_request_waiter =
          orbit_capture_service_base::CreateGrpcStartStopCaptureRequestWaiter(reader_writer);
  auto capture_options_or_error = start_stop_capture_request_waiter->WaitForStartCaptureRequest();
  if (capture_options_or_error.has_error()) {
    std::string error_msg = absl::StrFormat("An error occurred while waiting for start: %s",
                                            capture_options_or_error.error().message());
    ORBIT_ERROR("%s", error_msg);
    grpc_client_capture_event_collector.StopAndWait();
    TerminateCapture();
    return {grpc::StatusCode::INTERNAL, error_msg};
  }

  const orbit_grpc_protos::CaptureOptions& capture_options = capture_options_or_error.value();
  DoCapture(capture_options, start_stop_capture_request_waiter);
  return grpc::Status::OK;
}

}  // namespace orbit_linux_capture_service
