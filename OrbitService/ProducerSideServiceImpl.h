// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_
#define ORBIT_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_

#include "CaptureStartStopListener.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "producer_side_services.grpc.pb.h"

namespace orbit_service {

class ProducerSideServiceImpl final : public orbit_grpc_protos::ProducerSideService::Service,
                                      public CaptureStartStopListener {
 public:
  // This method causes the StartCaptureCommand to be sent to connected producers
  // (but if it's called multiple times in a row, the command will only be sent once).
  // CaptureEvents received from producers will be added to capture_event_buffer.
  void OnCaptureStartRequested(CaptureEventBuffer* capture_event_buffer) override;

  // This method causes the StopCaptureCommand to be sent to connected producers
  // (but if it's called multiple times in a row, the command will only be sent once).
  // The CaptureEventBuffer passed with OnCaptureStartRequested will no longer be filled.
  // This method blocks until all producers have notified they have sent all their CaptureEvents,
  // for a maximum time that can be specified with SetMaxWaitForAllCaptureEvents (default 10 s).
  void OnCaptureStopRequested() override;

  // This methods allows to specify a timeout for OnCaptureStopRequested, which blocks
  // until all CaptureEvents have been sent by the producers. The default is 10 seconds.
  void SetMaxWaitForAllCaptureEvents(absl::Duration duration) {
    max_wait_for_all_events_sent_ = duration;
  }

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
      bool* all_events_sent_received, std::atomic<bool>* exit_send_commands_thread);

  void ReceiveEventsThread(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                               orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
      bool* all_events_sent_received);

 private:
  absl::flat_hash_set<grpc::ServerContext*> server_contexts_;
  absl::Mutex server_contexts_mutex_;

  enum class CaptureStatus { kCaptureStarted, kCaptureStopping, kCaptureFinished };
  struct ServiceState {
    CaptureStatus capture_status = CaptureStatus::kCaptureFinished;
    int32_t producers_remaining = 0;
    bool exit_requested = false;
  } service_state_;
  absl::Mutex service_state_mutex_;

  CaptureEventBuffer* capture_event_buffer_ = nullptr;
  absl::Mutex capture_event_buffer_mutex_;

  absl::Duration max_wait_for_all_events_sent_ = absl::Seconds(10);
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PRODUCER_SIDE_SERVICE_IMPL_H_
