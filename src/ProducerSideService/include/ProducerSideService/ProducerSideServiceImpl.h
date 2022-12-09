// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_
#define ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <atomic>
#include <optional>

#include "CaptureServiceBase/CaptureStartStopListener.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/producer_side_services.grpc.pb.h"
#include "GrpcProtos/producer_side_services.pb.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_producer_side_service {

// This class implements the gRPC service ProducerSideService, and in particular its only RPC
// ReceiveCommandsAndSendEvents, through which producers of CaptureEvents connect to OrbitService.
// It also implements the CaptureStartStopListener interface, whose methods cause this service to
// notify the producers that a capture has been started (and that they can start sending
// CaptureEvents) or stopped (and that the producers should finish sending CaptureEvents).
// As OnCaptureStopRequested waits for the remaining CaptureEvents, SetMaxWaitForAllCaptureEventsMs
// allows to specify a timeout for that method.
// OnExitRequest disconnects all producers, preparing this service for shutdown.
class ProducerSideServiceImpl final : public orbit_grpc_protos::ProducerSideService::Service,
                                      public orbit_capture_service_base::CaptureStartStopListener {
 public:
  // This method causes the StartCaptureCommand to be sent to connected producers
  // (but if it's called multiple times in a row, the command will only be sent once).
  // CaptureEvents received from producers will be added to capture_event_buffer.
  void OnCaptureStartRequested(
      orbit_grpc_protos::CaptureOptions capture_options,
      orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor) override;

  // This method causes the StopCaptureCommand to be sent to connected producers
  // (but if it's called multiple times in a row, the command will only be sent once).
  // The CaptureEventBuffer passed with OnCaptureStartRequested will no longer be filled.
  // This method blocks until all producers have notified they have sent all their CaptureEvents,
  // for a maximum time that can be specified with SetMaxWaitForAllCaptureEventsMs (default 10 s).
  void OnCaptureStopRequested() override;

  // This methods allows to specify a timeout for OnCaptureStopRequested, which blocks
  // until all CaptureEvents have been sent by the producers. The default is 10 seconds.
  void SetMaxWaitForAllCaptureEventsMs(uint64_t ms) { max_wait_for_all_events_sent_ms_ = ms; }

  // This method forces to disconnect from connected producers and to terminate running threads.
  // It doesn't cause StopCaptureCommand to be sent, but producers will be able to handle
  // the fact that the connection was interrupted.
  // No OnCaptureStartRequested or OnCaptureStopRequested should be called afterwards.
  void OnExitRequest();

  grpc::Status ReceiveCommandsAndSendEvents(
      ::grpc::ServerContext* context,
      ::grpc::ServerReaderWriter< ::orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                                  ::orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream)
      override;

 private:
  void SendCommandsThread(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                               orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
      bool* all_events_sent_received, std::atomic<bool>* receive_events_thread_exited);

  void ReceiveEventsThread(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                               orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
      uint64_t producer_id, bool* all_events_sent_received);

  absl::flat_hash_set<grpc::ServerContext*> server_contexts_
      ABSL_GUARDED_BY(server_contexts_mutex_);
  absl::Mutex server_contexts_mutex_;

  enum class CaptureStatus { kCaptureStarted, kCaptureStopping, kCaptureFinished };
  struct ServiceState {
    CaptureStatus capture_status = CaptureStatus::kCaptureFinished;
    std::optional<orbit_grpc_protos::CaptureOptions> capture_options;
    int32_t producers_remaining = 0;
    bool exit_requested = false;
  } service_state_ ABSL_GUARDED_BY(service_state_mutex_);
  absl::Mutex service_state_mutex_;

  orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor_
      ABSL_GUARDED_BY(producer_event_processor_mutex_) = nullptr;
  absl::Mutex producer_event_processor_mutex_;

  std::atomic<uint64_t> producer_id_counter_ = orbit_grpc_protos::kExternalProducerStartingId;

  uint64_t max_wait_for_all_events_sent_ms_ = 10'000;
};

}  // namespace orbit_producer_side_service

#endif  // ORBIT_PRODUCER_SIDE_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_
