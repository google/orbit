// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/GrpcClientCaptureEventCollectorBuilder.h"

#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace orbit_capture_service {

// A `ClientCaptureEventCollectorBuilder` implementation to build `GrpcClientCaptureEventCollector`
// with `ServerReaderWriter` for the native orbit capture services.
class GrpcClientCaptureEventCollectorBuilder : public ClientCaptureEventCollectorBuilder {
 public:
  explicit GrpcClientCaptureEventCollectorBuilder(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
  BuildClientCaptureEventCollector() override {
    return std::make_unique<orbit_producer_event_processor::GrpcClientCaptureEventCollector>(
        reader_writer_);
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;
};

std::unique_ptr<ClientCaptureEventCollectorBuilder> CreateGrpcClientCaptureEventCollectorBuilder(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  return std::make_unique<GrpcClientCaptureEventCollectorBuilder>(reader_writer);
}

}  // namespace orbit_capture_service