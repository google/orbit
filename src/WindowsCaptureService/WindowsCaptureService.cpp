// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include "ApiLoader/EnableInTracee.h"
#include "CaptureServiceBase/CommonProducerCaptureEventBuilders.h"
#include "CaptureServiceBase/GrpcStartStopCaptureRequestWaiter.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "TracingHandler.h"

namespace orbit_windows_capture_service {

using orbit_capture_service_base::CaptureStartStopListener;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

grpc::Status WindowsCaptureService::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("WinCS::Capture");

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

  orbit_capture_service_base::GrpcStartStopCaptureRequestWaiter
      grpc_start_stop_capture_request_waiter{reader_writer};
  const CaptureOptions& capture_options =
      grpc_start_stop_capture_request_waiter.WaitForStartCaptureRequest();

  if (capture_options.enable_api()) {
    EnableApiInTracee(capture_options);
  }

  StartEventProcessing(capture_options);
  TracingHandler tracing_handler{producer_event_processor_.get()};
  tracing_handler.Start(capture_options);

  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(capture_options, producer_event_processor_.get());
  }

  StopCaptureReason stop_capture_reason =
      grpc_start_stop_capture_request_waiter.WaitForStopCaptureRequest();

  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStopRequested();
    ORBIT_LOG("CaptureStartStopListener stopped: one or more producers finished capturing");
  }

  if (capture_options.enable_api()) {
    DisableApiInTracee(capture_options);
  }

  tracing_handler.Stop();
  FinalizeEventProcessing(stop_capture_reason);

  TerminateCapture();

  return grpc::Status::OK;
}

void WindowsCaptureService::EnableApiInTracee(const CaptureOptions& capture_options) {
  auto result = orbit_api_loader::EnableApiInTracee(capture_options);
  if (!result.has_error()) return;

  std::string error = absl::StrFormat("Could not enable Orbit API: %s", result.error().message());
  ORBIT_ERROR("%s", error);
  producer_event_processor_->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service_base::CreateErrorEnablingOrbitApiEvent(orbit_base::CaptureTimestampNs(),
                                                                   error));
}

void WindowsCaptureService::DisableApiInTracee(const CaptureOptions& capture_options) {
  auto result = orbit_api_loader::DisableApiInTracee(capture_options);
  if (!result.has_error()) return;

  std::string error = absl::StrFormat("Could not disable Orbit API: %s", result.error().message());
  ORBIT_ERROR("%s", error);
  producer_event_processor_->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service_base::CreateWarningEvent(orbit_base::CaptureTimestampNs(), error));
}

}  // namespace orbit_windows_capture_service
