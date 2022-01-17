// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/CaptureService.h"

#include <absl/time/time.h>

#include "CaptureService/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::GrpcClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service {

CaptureService::CaptureService() {
  // We want to estimate clock resolution once, not at the beginning of every capture.
  clock_resolution_ns_ = orbit_base::EstimateAndLogClockResolution();
}

void CaptureService::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  CHECK(new_insertion);
}

void CaptureService::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  CHECK(was_removed);
}

grpc::Status CaptureService::InitializeCapture(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  {
    absl::MutexLock lock(&capture_mutex_);
    if (is_capturing_) {
      return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                          "Cannot start capture because another capture is already in progress");
    }
    is_capturing_ = true;
  }

  grpc_client_capture_event_collector_ =
      std::make_unique<GrpcClientCaptureEventCollector>(reader_writer);

  producer_event_processor_ =
      ProducerEventProcessor::Create(grpc_client_capture_event_collector_.get());

  return grpc::Status::OK;
}

void CaptureService::TerminateCapture() {
  producer_event_processor_.reset();
  grpc_client_capture_event_collector_.reset();
  capture_start_timestamp_ns_ = 0;

  absl::MutexLock lock(&capture_mutex_);
  is_capturing_ = false;
}

CaptureRequest CaptureService::WaitForStartCaptureRequestFromClient(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer) {
  CaptureRequest request;
  // This call is blocking.
  reader_writer->Read(&request);

  ORBIT_LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
  return request;
}

void CaptureService::WaitForStopCaptureRequestFromClient(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer) {
  orbit_grpc_protos::CaptureRequest request;
  // The client asks for the capture to be stopped by calling WritesDone. At that point, this call
  // to Read will return false. In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");
}

void CaptureService::StartEventProcessing(const CaptureOptions& capture_options) {
  // These are not in precise sync but they do not have to be.
  absl::Time capture_start_time = absl::Now();
  capture_start_timestamp_ns_ = orbit_base::CaptureTimestampNs();

  producer_event_processor_->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service::CreateCaptureStartedEvent(capture_options, capture_start_time,
                                                       capture_start_timestamp_ns_));

  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                          orbit_capture_service::CreateClockResolutionEvent(
                                              capture_start_timestamp_ns_, clock_resolution_ns_));
}

void CaptureService::FinalizeEventProcessing(StopCaptureReason stop_capture_reason) {
  ProducerCaptureEvent capture_finished;
  switch (stop_capture_reason) {
    case StopCaptureReason::kClientStop:
      capture_finished = CreateSuccessfulCaptureFinishedEvent();
      break;
    case StopCaptureReason::kMemoryWatchdog:
      capture_finished = CreateMemoryThresholdExceededCaptureFinishedEvent();
      break;
  }
  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                          std::move(capture_finished));

  grpc_client_capture_event_collector_->StopAndWait();
  ORBIT_LOG("Finished handling gRPC call to Capture: all capture data has been sent");
}

}  // namespace orbit_capture_service
