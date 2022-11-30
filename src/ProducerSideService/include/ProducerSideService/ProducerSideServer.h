// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVER_H_
#define ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVER_H_

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>
#include <string_view>

#include "CaptureServiceBase/CaptureStartStopListener.h"
#include "GrpcProtos/capture.pb.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "ProducerSideService/ProducerSideServiceImpl.h"

namespace orbit_producer_side_service {

// Wrapper around a grpc::Server that registers the service ProducerSideServiceImpl
// and listens on a socket.
class ProducerSideServer final : public orbit_capture_service_base::CaptureStartStopListener {
 public:
  bool BuildAndStart(std::string_view uri);
  void ShutdownAndWait();

  void OnCaptureStartRequested(
      orbit_grpc_protos::CaptureOptions capture_options,
      orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor) override;
  void OnCaptureStopRequested() override;

 private:
  ProducerSideServiceImpl producer_side_service_;
  std::unique_ptr<grpc::Server> server_;
};

}  // namespace orbit_producer_side_service

#endif  // ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVER_H_
