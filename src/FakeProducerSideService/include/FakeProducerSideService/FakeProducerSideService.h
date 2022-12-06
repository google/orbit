// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_PRODUCER_SIDE_SERVICE_FAKE_PRODUCER_SIDE_SERVICE_H_
#define FAKE_PRODUCER_SIDE_SERVICE_FAKE_PRODUCER_SIDE_SERVICE_H_

#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "GrpcProtos/producer_side_services.grpc.pb.h"

namespace orbit_fake_producer_side_service {

// This class fakes a ProducerSideService for use in tests.
// SendStartCaptureCommand, SendStopCaptureCommand, SendCaptureFinishedCommand should not be called
// concurrently.
class FakeProducerSideService : public orbit_grpc_protos::ProducerSideService::Service {
 public:
  grpc::Status ReceiveCommandsAndSendEvents(
      ::grpc::ServerContext* context,
      ::grpc::ServerReaderWriter< ::orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                                  ::orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream)
      override {
    if (!rpc_allowed_) {
      return grpc::Status::CANCELLED;
    }
    {
      absl::WriterMutexLock lock{&context_and_stream_mutex_};
      EXPECT_EQ(context_, nullptr);
      EXPECT_EQ(stream_, nullptr);
      context_ = context;
      stream_ = stream;
    }

    while (true) {
      orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest request;
      {
        absl::ReaderMutexLock lock{&context_and_stream_mutex_};
        if (!stream->Read(&request)) break;
      }
      EXPECT_NE(request.event_case(),
                orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET);
      switch (request.event_case()) {
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kBufferedCaptureEvents: {
          std::vector<orbit_grpc_protos::ProducerCaptureEvent> capture_events_received{
              request.buffered_capture_events().capture_events().begin(),
              request.buffered_capture_events().capture_events().end()};
          OnCaptureEventsReceived(capture_events_received);
        } break;
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kAllEventsSent:
          OnAllEventsSentReceived();
          break;
        case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET:
          break;
      }
    }

    {
      absl::WriterMutexLock lock{&context_and_stream_mutex_};
      context_ = nullptr;
      stream_ = nullptr;
    }
    return grpc::Status::OK;
  }

  void SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions capture_options) {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    *command.mutable_start_capture_command()->mutable_capture_options() =
        std::move(capture_options);
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void SendStopCaptureCommand() {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    command.mutable_stop_capture_command();
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void SendCaptureFinishedCommand() {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
    command.mutable_capture_finished_command();
    bool written = stream_->Write(command);
    EXPECT_TRUE(written);
  }

  void FinishAndDisallowRpc() {
    rpc_allowed_ = false;
    {
      absl::ReaderMutexLock lock{&context_and_stream_mutex_};
      if (context_ != nullptr) {
        EXPECT_NE(stream_, nullptr);
        context_->TryCancel();
      }
    }
    {
      absl::WriterMutexLock lock{&context_and_stream_mutex_};
      context_ = nullptr;
      stream_ = nullptr;
    }
  }

  void ReAllowRpc() { rpc_allowed_ = true; }

  MOCK_METHOD(void, OnCaptureEventsReceived,
              (absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events), ());
  MOCK_METHOD(void, OnAllEventsSentReceived, (), ());

 private:
  grpc::ServerContext* context_ ABSL_GUARDED_BY(context_and_stream_mutex_) = nullptr;
  grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                           orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream_
      ABSL_GUARDED_BY(context_and_stream_mutex_) = nullptr;
  absl::Mutex context_and_stream_mutex_;
  std::atomic<bool> rpc_allowed_ = true;
};

}  // namespace orbit_fake_producer_side_service

#endif  // FAKE_PRODUCER_SIDE_SERVICE_FAKE_PRODUCER_SIDE_SERVICE_H_
