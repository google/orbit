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
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "services.grpc.pb.h"
#include "services.pb.h"

namespace orbit_capture_service {

// CaptureService is an interface derived from the grpc capture service. It holds common
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
  void EstimateAndLogClockResolution();

  ErrorMessageOr<void> InitializeCapture(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);
  void TerminateCapture();

  void WaitForStartCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer,
      orbit_grpc_protos::CaptureRequest& request);

  void WaitForEndCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer,
      orbit_grpc_protos::CaptureRequest& request);

  void StartEventProcessing(const orbit_grpc_protos::CaptureOptions& capture_options);
  void FinalizeEventProcessing();

 protected:
  mutable absl::Mutex capture_mutex_;
  bool is_capturing = false;

  std::unique_ptr<orbit_producer_event_processor::GrpcClientCaptureEventCollector>
      grpc_client_capture_event_collector_;
  std::unique_ptr<orbit_producer_event_processor::ProducerEventProcessor> producer_event_processor_;

  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners_;

  uint64_t clock_resolution_ns_ = 0;
  uint64_t capture_start_timestamp_ns_ = 0;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_H_
