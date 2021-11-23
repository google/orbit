// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_SERVICE_H_
#define CAPTURE_SERVICE_CAPTURE_SERVICE_H_

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>

#include <memory>

#include "CaptureStartStopListener.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service {

// CaptureService is an abstract class derived from the grpc capture service. It holds common
// functionality shared by the platform-specific capture services.
class CaptureService : public orbit_grpc_protos::CaptureService::Service {
 public:
  virtual ~CaptureService() = default;
  CaptureService();

  virtual grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) = 0;

  void AddCaptureStartStopListener(CaptureStartStopListener* listener);
  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener);

 protected:
  grpc::Status InitializeCapture(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);
  void TerminateCapture();

  orbit_grpc_protos::CaptureRequest WaitForStartCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);

  void WaitForEndCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);

  void StartEventProcessing(const orbit_grpc_protos::CaptureOptions& capture_options);
  void FinalizeEventProcessing();

  std::unique_ptr<orbit_producer_event_processor::GrpcClientCaptureEventCollector>
      grpc_client_capture_event_collector_;
  std::unique_ptr<orbit_producer_event_processor::ProducerEventProcessor> producer_event_processor_;

  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners_;
  uint64_t capture_start_timestamp_ns_ = 0;

 private:
  void EstimateAndLogClockResolution();

  uint64_t clock_resolution_ns_ = 0;
  absl::Mutex capture_mutex_;
  bool is_capturing ABSL_GUARDED_BY(capture_mutex_) = false;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_H_
