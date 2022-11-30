// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerSideService/ProducerSideServer.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>

#include <string>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_producer_side_service {

bool ProducerSideServer::BuildAndStart(std::string_view uri) {
  ORBIT_CHECK(server_ == nullptr);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(std::string{uri}, grpc::InsecureServerCredentials());

  builder.RegisterService(&producer_side_service_);

  server_ = builder.BuildAndStart();
  if (server_ == nullptr) {
    return false;
  }

  return true;
}

void ProducerSideServer::ShutdownAndWait() {
  ORBIT_CHECK(server_ != nullptr);
  producer_side_service_.OnExitRequest();
  server_->Shutdown();
  server_->Wait();
}

void ProducerSideServer::OnCaptureStartRequested(
    orbit_grpc_protos::CaptureOptions capture_options,
    orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor) {
  producer_side_service_.OnCaptureStartRequested(std::move(capture_options),
                                                 producer_event_processor);
}

void ProducerSideServer::OnCaptureStopRequested() {
  producer_side_service_.OnCaptureStopRequested();
}

}  // namespace orbit_producer_side_service
