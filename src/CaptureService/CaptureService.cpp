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
  meta_data_.clock_resolution_ns = orbit_base::EstimateAndLogClockResolution();
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

  meta_data_.Init(std::make_unique<GrpcClientCaptureEventCollector>(reader_writer));

  return grpc::Status::OK;
}

void CaptureService::TerminateCapture() {
  meta_data_.Reset();

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

}  // namespace orbit_capture_service
