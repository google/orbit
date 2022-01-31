// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/Capturer.h"

#include <absl/time/time.h>

#include "CaptureService/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service {

Capturer::Capturer() {
  // We want to estimate clock resolution once, not at the beginning of every capture.
  clock_resolution_ns_ = orbit_base::EstimateAndLogClockResolution();
}

void Capturer::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  ORBIT_CHECK(new_insertion);
}

void Capturer::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  ORBIT_CHECK(was_removed);
}

ErrorMessageOr<void> Capturer::InitializeCapture(
    std::unique_ptr<ClientCaptureEventCollector> client_capture_event_collector) {
  {
    absl::MutexLock lock(&capture_mutex_);
    if (is_capturing_) {
      return ErrorMessage("Cannot start capture because another capture is already in progress");
    }
    is_capturing_ = true;
  }

  ORBIT_CHECK(client_capture_event_collector != nullptr);
  client_capture_event_collector_.reset(client_capture_event_collector.release());
  producer_event_processor_ = ProducerEventProcessor::Create(client_capture_event_collector_.get());
  return outcome::success();
}

void Capturer::TerminateCapture() {
  producer_event_processor_.reset();
  client_capture_event_collector_.reset();
  capture_start_timestamp_ns_ = 0;

  absl::MutexLock lock(&capture_mutex_);
  is_capturing_ = false;
}

void Capturer::StartEventProcessing(const CaptureOptions& capture_options) {
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

void Capturer::FinalizeEventProcessing(StopCaptureReason stop_capture_reason) {
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
