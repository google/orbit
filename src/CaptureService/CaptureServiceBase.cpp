// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/CaptureServiceBase.h"

#include <absl/time/time.h>

#include "CaptureService/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service {

void CaptureServiceBase::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  ORBIT_CHECK(new_insertion);
}

void CaptureServiceBase::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  ORBIT_CHECK(was_removed);
}

CaptureServiceBase::CaptureInitializationResult CaptureServiceBase::InitializeCapture(
    ClientCaptureEventCollectorBuilder* client_capture_event_collector_builder) {
  {
    absl::MutexLock lock(&capture_mutex_);
    if (is_capturing_) {
      return CaptureInitializationResult::kAlreadyInProgress;
    }
    is_capturing_ = true;
  }

  client_capture_event_collector_ =
      client_capture_event_collector_builder->BuildClientCaptureEventCollector();
  producer_event_processor_ = ProducerEventProcessor::Create(client_capture_event_collector_.get());
  return CaptureInitializationResult::kSuccess;
}

void CaptureServiceBase::TerminateCapture() {
  producer_event_processor_.reset();
  client_capture_event_collector_.reset();
  capture_start_timestamp_ns_ = 0;

  absl::MutexLock lock(&capture_mutex_);
  is_capturing_ = false;
}

void CaptureServiceBase::StartEventProcessing(const CaptureOptions& capture_options) {
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

void CaptureServiceBase::FinalizeEventProcessing(StopCaptureReason stop_capture_reason) {
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

  client_capture_event_collector_->StopAndWait();
  ORBIT_LOG("Finished processing CaptureFinisedEvent");
}

}  // namespace orbit_capture_service
