// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_SERVICE_H_
#define CAPTURE_SERVICE_CAPTURE_SERVICE_H_

#include <absl/container/flat_hash_set.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>

#include "CaptureStartStopListener.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service {

class CaptureService : public orbit_grpc_protos::CaptureService::Service {
 public:
  virtual ~CaptureService() = default;
  CaptureService() {
    // We want to estimate clock resolution once, not at the beginning of every capture.
    EstimateAndLogClockResolution();
  }

  virtual grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) = 0;

  void AddCaptureStartStopListener(CaptureStartStopListener* listener);
  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener);

 protected:
  std::atomic<bool> is_capturing = false;
  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners_;

  uint64_t clock_resolution_ns_ = 0;
  void EstimateAndLogClockResolution();
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_H_
