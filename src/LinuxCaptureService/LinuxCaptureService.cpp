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

  std::shared_ptr<orbit_capture_service_base::StartStopCaptureRequestWaiter>
      start_stop_capture_request_waiter =
          orbit_capture_service_base::CreateGrpcStartStopCaptureRequestWaiter(reader_writer);

  CaptureServiceBase::CaptureInitializationResult initialization_result =
      DoCapture(&grpc_client_capture_event_collector, start_stop_capture_request_waiter);
  switch (initialization_result) {
    case CaptureInitializationResult::kSuccess:
      return grpc::Status::OK;
    case CaptureInitializationResult::kAlreadyInProgress:
      return {grpc::StatusCode::ALREADY_EXISTS,
              "Cannot start capture because another capture is already in progress"};
    case CaptureInitializationResult::kFailureWhileWaitingForStart:
      return {grpc::StatusCode::INTERNAL,
              "An error occurred while waiting for start capture request"};
  }

  ORBIT_UNREACHABLE();
}

}  // namespace orbit_linux_capture_service
