// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/WindowsCaptureService.h"

#include <absl/time/time.h>
#include <stdint.h>

#include <utility>
#include <vector>

#include "CaptureService/ProducerCaptureEventBuilder.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "TracingHandler.h"
#include "capture.pb.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ProducerEventProcessor;

using orbit_producer_event_processor::GrpcClientCaptureEventCollector;

grpc::Status WindowsCaptureService::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("WinCS::Capture");
  if (is_capturing) {
    ERROR("Cannot start capture because another capture is already in progress");
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "Cannot start capture because another capture is already in progress.");
  }
  is_capturing = true;

  GrpcClientCaptureEventCollector client_capture_event_collector{reader_writer};
  std::unique_ptr<ProducerEventProcessor> producer_event_processor =
      ProducerEventProcessor::Create(&client_capture_event_collector);
  TracingHandler tracing_handler{producer_event_processor.get()};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");

  const CaptureOptions& capture_options = request.capture_options();

  // These are not in precise sync but they do not have to be.
  absl::Time capture_start_time = absl::Now();
  uint64_t capture_start_timestamp_ns = orbit_base::CaptureTimestampNs();

  producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      orbit_capture_service::CreateCaptureStartedEvent(capture_options, capture_start_time,
                                                       capture_start_timestamp_ns));

  tracing_handler.Start(capture_options);

  // The client asks for the capture to be stopped by calling WritesDone. At that point, this call
  // to Read will return false. In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");

  tracing_handler.Stop();
  LOG("Windows TracingHandler stopped: etw tracing is done");

  producer_event_processor->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                         orbit_capture_service::CreateCaptureFinishedEvent());

  client_capture_event_collector.StopAndWait();
  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  is_capturing = false;
  return grpc::Status::OK;
}

}  // namespace orbit_windows_capture_service
