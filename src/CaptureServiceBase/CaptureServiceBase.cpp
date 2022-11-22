// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CaptureServiceBase.h"

#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <utility>

#include "CaptureServiceBase/CaptureStartStopListener.h"
#include "CaptureServiceBase/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service_base {

void CaptureServiceBase::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  ORBIT_CHECK(new_insertion);
}

void CaptureServiceBase::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  ORBIT_CHECK(was_removed);
}

CaptureServiceBase::CaptureInitializationResult CaptureServiceBase::InitializeCapture(
    ClientCaptureEventCollector* client_capture_event_collector) {
  ORBIT_CHECK(client_capture_event_collector != nullptr);

  {
    absl::MutexLock lock(&capture_mutex_);
    if (is_capturing_) {
      return CaptureInitializationResult::kAlreadyInProgress;
    }
    is_capturing_ = true;
  }

  client_capture_event_collector_ = client_capture_event_collector;
  producer_event_processor_ = ProducerEventProcessor::Create(client_capture_event_collector_);
  return CaptureInitializationResult::kSuccess;
}

void CaptureServiceBase::TerminateCapture() {
  producer_event_processor_.reset();
  client_capture_event_collector_ = nullptr;
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
      CreateCaptureStartedEvent(capture_options, capture_start_time, capture_start_timestamp_ns_));

  producer_event_processor_->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      CreateClockResolutionEvent(capture_start_timestamp_ns_, clock_resolution_ns_));
}

void CaptureServiceBase::FinalizeEventProcessing(
    StopCaptureReason stop_capture_reason,
    CaptureFinished::ProcessState target_process_state_after_capture,
    CaptureFinished::TerminationSignal target_process_termination_signal) {
  ProducerCaptureEvent capture_finished;
  switch (stop_capture_reason) {
    case StopCaptureReason::kUnknown:
      capture_finished = CreateFailedCaptureFinishedEvent("Capture stopped due to unknown reason.");
      break;
    case StopCaptureReason::kClientStop:
    case StopCaptureReason::kGuestOrcStop:
      capture_finished = CreateSuccessfulCaptureFinishedEvent();
      break;
    case StopCaptureReason::kMemoryWatchdog:
      capture_finished =
          CreateInterruptedByServiceCaptureFinishedEvent("OrbitService was using too much memory.");
      break;
    case StopCaptureReason::kExceededMaxDurationLimit:
      capture_finished = CreateInterruptedByServiceCaptureFinishedEvent(
          "Capture duration exceeded the maximum duration limit.");
      break;
    case StopCaptureReason::kGuestOrcConnectionFailure:
      capture_finished = CreateFailedCaptureFinishedEvent("Connection with GuestOrc failed.");
      break;
    case StopCaptureReason::kUploadFailure:
      capture_finished = CreateFailedCaptureFinishedEvent("Upload failed early.");
      break;
  }

  capture_finished.mutable_capture_finished()->set_target_process_state_after_capture(
      target_process_state_after_capture);
  capture_finished.mutable_capture_finished()->set_target_process_termination_signal(
      target_process_termination_signal);

  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                          std::move(capture_finished));

  client_capture_event_collector_->StopAndWait();
  ORBIT_LOG("Finished processing CaptureFinishedEvent");
}

}  // namespace orbit_capture_service_base
