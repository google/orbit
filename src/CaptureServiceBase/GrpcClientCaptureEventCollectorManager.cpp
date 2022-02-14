// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/GrpcClientCaptureEventCollectorManager.h"

#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::GrpcClientCaptureEventCollector;

namespace orbit_capture_service_base {

// A `ClientCaptureEventCollectorManager` implementation to build `GrpcClientCaptureEventCollector`
// with `ServerReaderWriter` for the native orbit capture services.
class GrpcClientCaptureEventCollectorManager : public ClientCaptureEventCollectorManager {
 public:
  explicit GrpcClientCaptureEventCollectorManager(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : grpc_client_capture_event_collector_{
            std::make_unique<GrpcClientCaptureEventCollector>(reader_writer)} {}

  [[nodiscard]] ClientCaptureEventCollector* GetClientCaptureEventCollector() override {
    return grpc_client_capture_event_collector_.get();
  }

 private:
  std::unique_ptr<GrpcClientCaptureEventCollector> grpc_client_capture_event_collector_;
};

std::unique_ptr<ClientCaptureEventCollectorManager> CreateGrpcClientCaptureEventCollectorManager(
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  return std::make_unique<GrpcClientCaptureEventCollectorManager>(reader_writer);
}

}  // namespace orbit_capture_service_base