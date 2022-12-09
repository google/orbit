// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_EVENT_PRODUCER_CAPTURE_EVENT_PRODUCER_H_
#define CAPTURE_EVENT_PRODUCER_CAPTURE_EVENT_PRODUCER_H_

#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <atomic>
#include <memory>
#include <thread>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/producer_side_services.grpc.pb.h"
#include "GrpcProtos/producer_side_services.pb.h"

namespace orbit_capture_event_producer {

// This abstract class offers the subclasses methods
// to connect and communicate with a ProducerSideService.
class CaptureEventProducer {
 public:
  virtual ~CaptureEventProducer() = default;

  [[nodiscard]] bool IsCapturing() {
    return last_command_ ==
           orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand;
  }

  // This method allows to specify how frequently a reconnection with the service should
  // be attempted when the connection fails or is interrupted. The default is 4 seconds.
  void SetReconnectionDelayMs(uint64_t ms) { reconnection_delay_ms_ = ms; }

 protected:
  // This method establishes the connection with ProducerSideService. If a connection fails or
  // is interrupted, the class will keep trying to (re)connect, until ShutdownAndWait is called.
  // Subclasses that extend this method by overriding it must also call the overridden method.
  virtual void BuildAndStart(const std::shared_ptr<grpc::Channel>& channel);
  // This method causes to disconnect from ProducerSideService or to stop trying to reconnect to it.
  // Subclasses that extend this method by overriding it must also call the overridden method.
  virtual void ShutdownAndWait();

  // Subclasses need to override this method to be notified of a request to start a capture.
  // This is also the chance for the subclasses to read or store the CaptureOptions.
  virtual void OnCaptureStart(orbit_grpc_protos::CaptureOptions capture_options) = 0;
  // Subclasses need to override this method to be notified of a request to stop the capture.
  virtual void OnCaptureStop() = 0;
  // Subclasses need to override this method to be notified that the current capture has finished.
  virtual void OnCaptureFinished() = 0;

  // Subclasses can use this method to send a batch of CaptureEvents to the ProducerSideService.
  // A full ReceiveCommandsAndSendEventsRequest with event_case() == kBufferedCaptureEvents
  // needs to be passed to avoid an extra copy from a BufferedCaptureEvents message.
  [[nodiscard]] bool SendCaptureEvents(
      const orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest& send_events_request);
  // Subclasses should use this method to notify the ProducerSideService that
  // they have sent all their CaptureEvents after the capture has been stopped.
  [[nodiscard]] bool NotifyAllEventsSent();

 private:
  void ConnectAndReceiveCommandsThread();

  std::unique_ptr<orbit_grpc_protos::ProducerSideService::Stub> producer_side_service_stub_;
  std::thread connect_and_receive_commands_thread_;

  std::unique_ptr<grpc::ClientContext> context_;
  std::unique_ptr<grpc::ClientReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest,
                                           orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse>>
      stream_;
  absl::Mutex context_and_stream_mutex_;

  std::atomic<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::CommandCase> last_command_ =
      orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;

  bool shutdown_requested_ = false;
  absl::Mutex shutdown_requested_mutex_;

  std::atomic<uint64_t> reconnection_delay_ms_ = 4000;
};

}  // namespace orbit_capture_event_producer

#endif  // CAPTURE_EVENT_PRODUCER_CAPTURE_EVENT_PRODUCER_H_
