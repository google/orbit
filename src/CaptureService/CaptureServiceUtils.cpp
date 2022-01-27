// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/CaptureServiceUtils.h"

#include <absl/time/time.h>

#include "CaptureService/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Profiling.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service {

void StartEventProcessing(const CaptureOptions& capture_options,
                          CaptureServiceMetaData& meta_data) {
  // These are not in precise sync but they do not have to be.
  absl::Time capture_start_time = absl::Now();
  meta_data.capture_start_timestamp_ns = orbit_base::CaptureTimestampNs();

  meta_data.producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service::CreateCaptureStartedEvent(capture_options, capture_start_time,
                                                       meta_data.capture_start_timestamp_ns));

  meta_data.producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service::CreateClockResolutionEvent(meta_data.capture_start_timestamp_ns,
                                                        meta_data.clock_resolution_ns));
}

void FinalizeEventProcessing(StopCaptureReason stop_capture_reason,
                             CaptureServiceMetaData& meta_data) {
  ProducerCaptureEvent capture_finished;
  switch (stop_capture_reason) {
    case StopCaptureReason::kClientStop:
      capture_finished = CreateSuccessfulCaptureFinishedEvent();
      break;
    case StopCaptureReason::kMemoryWatchdog:
      capture_finished = CreateMemoryThresholdExceededCaptureFinishedEvent();
      break;
  }
  meta_data.producer_event_processor->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                                   std::move(capture_finished));

  meta_data.client_capture_event_collector->StopAndWait();
  ORBIT_LOG("Finished processing CaptureFinisedEvent");
}

}  // namespace orbit_capture_service
