// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <google/protobuf/arena.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/channel_arguments.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include "CaptureEventProducer/LockFreeBufferCaptureEventProducer.h"
#include "FakeProducerSideService/FakeProducerSideService.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_event_producer {

namespace {

class LockFreeBufferCaptureEventProducerImpl
    : public LockFreeBufferCaptureEventProducer<std::string> {
 protected:
  orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      std::string&& /*intermediate_event*/, google::protobuf::Arena* arena) override {
    return google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
  }
};

class LockFreeBufferCaptureEventProducerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fake_service_.emplace();

    grpc::ServerBuilder builder;
    builder.RegisterService(&fake_service_.value());
    fake_server_ = builder.BuildAndStart();
    ASSERT_NE(fake_server_, nullptr);

    std::shared_ptr<grpc::Channel> channel =
        fake_server_->InProcessChannel(grpc::ChannelArguments{});

    buffer_producer_.emplace();
    buffer_producer_->BuildAndStart(channel);

    // Leave some time for the ReceiveCommandsAndSendEvents RPC to actually happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void TearDown() override {
    // Leave some time for all pending communication to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    buffer_producer_->ShutdownAndWait();
    buffer_producer_.reset();

    fake_service_->FinishAndDisallowRpc();
    fake_server_->Shutdown();
    fake_server_->Wait();

    fake_service_.reset();
    fake_server_.reset();
  }

  std::optional<orbit_fake_producer_side_service::FakeProducerSideService> fake_service_;
  std::unique_ptr<grpc::Server> fake_server_;
  std::optional<LockFreeBufferCaptureEventProducerImpl> buffer_producer_;
};

constexpr std::chrono::milliseconds kWaitMessagesSentDuration{25};

}  // namespace

TEST_F(LockFreeBufferCaptureEventProducerTest, EnqueueIntermediateEventIfCapturing) {
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
}

TEST_F(LockFreeBufferCaptureEventProducerTest, EnqueueIntermediateEvent) {
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  buffer_producer_->EnqueueIntermediateEvent("");
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  buffer_producer_->EnqueueIntermediateEvent("");
  {
    std::string intermediate_event_passed_by_const_ref;
    buffer_producer_->EnqueueIntermediateEvent(intermediate_event_passed_by_const_ref);
  }
  buffer_producer_->EnqueueIntermediateEvent("");
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  buffer_producer_->EnqueueIntermediateEvent("");
  buffer_producer_->EnqueueIntermediateEvent("");
}

TEST_F(LockFreeBufferCaptureEventProducerTest, DuplicatedCommands) {
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  // This should have no effect.
  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 2));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 5);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  // This should have no effect.
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  // This should have no effect.
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
}

TEST_F(LockFreeBufferCaptureEventProducerTest, ServiceDisconnects) {
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  // Disconnect.
  fake_service_->FinishAndDisallowRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
}

TEST_F(LockFreeBufferCaptureEventProducerTest, DisconnectAndReconnect) {
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  // Reduce reconnection delay before disconnecting.
  static constexpr uint64_t kReconnectionDelayMs = 50;
  buffer_producer_->SetReconnectionDelayMs(kReconnectionDelayMs);

  // Disconnect.
  fake_service_->FinishAndDisallowRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  // Wait for reconnection.
  fake_service_->ReAllowRpc();
  std::this_thread::sleep_for(std::chrono::milliseconds{2 * kReconnectionDelayMs});

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(buffer_producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 2));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  EXPECT_TRUE(buffer_producer_->EnqueueIntermediateEventIfCapturing([] { return ""; }));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 5);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(buffer_producer_->IsCapturing());
}

}  // namespace orbit_capture_event_producer
