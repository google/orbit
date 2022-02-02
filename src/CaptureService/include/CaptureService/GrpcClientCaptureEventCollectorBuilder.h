// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_
#define CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_

#include <grpcpp/grpcpp.h>

#include "CaptureService/ClientCaptureEventCollectorBuilder.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

namespace orbit_capture_service {

// A `ClientCaptureEventCollectorBuilder` implementation to build `GrpcClientCaptureEventCollector`
// with `ServerReaderWriter` for the native orbit capture services.
class GrpcClientCaptureEventCollectorBuilder : public ClientCaptureEventCollectorBuilder {
 public:
  explicit GrpcClientCaptureEventCollectorBuilder(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
  BuildClientCaptureEventCollector() override {
    return std::make_unique<orbit_producer_event_processor::GrpcClientCaptureEventCollector>(
        reader_writer_);
  }

 private:
  grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
      reader_writer_;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_