// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_FAKE_PRODUCER_SIDE_SERVICE_H_
#define ORBIT_PRODUCER_FAKE_PRODUCER_SIDE_SERVICE_H_

#include "grpcpp/grpcpp.h"
#include "producer_side_services.grpc.pb.h"

namespace orbit_producer {

// This class fakes a ProducerSideService for use in tests.
class FakeProducerSideService : public orbit_grpc_protos::ProducerSideService::Service {
 public:
  grpc::Status ReceiveCommandsAndSendEvents(
      ::grpc::ServerContext* context,
      ::grpc::ServerReaderWriter< ::orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                                  ::orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream)
      override {
    EXPECT_EQ(context_, nullptr);
    EXPECT_EQ(stream_, nullptr);
    if (!rpc_allowed_) {
      return grpc::Status::CANCELLED;
    }
    context_ = context;
    stream_ = stream;

    orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest request;
    while (stream->Read(&request)) {
      EXPECT_NE(request.event_case(),
                orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET);
      switch (request.event_case()) {
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kBufferedCaptureEvents:
          OnCaptureEventsReceived(request.buffered_capture_events().capture_events_size());
          break;
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kAllEventsSent:
          OnAllEventsSentReceived();
          break;
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET:
          break;
      }
    }

    context_ = nullptr;
    stream_ = nullptr;
    return grpc::Status::OK;
  }

  void SendStartCaptureCommand() {
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    command.mutable_start_capture_command();
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void SendStopCaptureCommand() {
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    command.mutable_stop_capture_command();
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void SendCaptureFinishedCommand() {
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    command.mutable_capture_finished_command();
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void FinishAndDisallowRpc() {
    rpc_allowed_ = false;
    if (context_ != nullptr) {
      EXPECT_NE(stream_, nullptr);
      context_->TryCancel();
      context_ = nullptr;
    }
    stream_ = nullptr;
  }

  void ReAllowRpc() { rpc_allowed_ = true; }

  MOCK_METHOD(void, OnCaptureEventsReceived, (int32_t count), ());
  MOCK_METHOD(void, OnAllEventsSentReceived, (), ());

 private:
  grpc::ServerContext* context_ = nullptr;
  grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                           orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream_ =
      nullptr;
  std::atomic<bool> rpc_allowed_ = true;
};

}  // namespace orbit_producer

#endif  // ORBIT_PRODUCER_FAKE_PRODUCER_SIDE_SERVICE_H_
