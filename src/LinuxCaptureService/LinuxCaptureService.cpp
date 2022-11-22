// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxCaptureService/LinuxCaptureService.h"

#include <memory>

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "CaptureServiceBase/GrpcStartStopCaptureRequestWaiter.h"
#include "GrpcProtos/capture.pb.h"
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

  auto grpc_start_stop_capture_request_waiter =
      std::make_shared<orbit_capture_service_base::GrpcStartStopCaptureRequestWaiter>(
          reader_writer);
  const orbit_grpc_protos::CaptureOptions& capture_options =
      grpc_start_stop_capture_request_waiter->WaitForStartCaptureRequest();
  DoCapture(capture_options, grpc_start_stop_capture_request_waiter);

  return grpc::Status::OK;
}

}  // namespace orbit_linux_capture_service
