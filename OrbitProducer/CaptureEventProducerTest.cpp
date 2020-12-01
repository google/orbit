// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

#include "FakeProducerSideService.h"
#include "OrbitProducer/CaptureEventProducer.h"
#include "absl/strings/str_format.h"
#include "grpcpp/grpcpp.h"
#include "producer_side_services.grpc.pb.h"

namespace orbit_producer {

namespace {

class CaptureEventProducerImpl : public CaptureEventProducer {
 public:
  // Override and forward these methods to make them public.
  void BuildAndStart(const std::shared_ptr<grpc::Channel>& channel) override {
    CaptureEventProducer::BuildAndStart(channel);
  }

  void ShutdownAndWait() override { CaptureEventProducer::ShutdownAndWait(); }

  // Hide and forward these methods to make them public.
  [[nodiscard]] bool SendCaptureEvents(
      const orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest& send_events_request) {
    return CaptureEventProducer::SendCaptureEvents(send_events_request);
  }

  [[nodiscard]] bool NotifyAllEventsSent() { return CaptureEventProducer::NotifyAllEventsSent(); }

  MOCK_METHOD(void, OnCaptureStart, (), (override));
  MOCK_METHOD(void, OnCaptureStop, (), (override));
};

class CaptureEventProducerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fake_service.emplace();

    grpc::ServerBuilder builder;
    builder.RegisterService(&*fake_service);
    fake_server = builder.BuildAndStart();
    ASSERT_NE(fake_server, nullptr);

    std::shared_ptr<grpc::Channel> channel =
        fake_server->InProcessChannel(grpc::ChannelArguments{});

    producer.emplace();
    producer->BuildAndStart(channel);

    // Leave some time for the ReceiveCommandsAndSendEvents RPC to actually happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void TearDown() override {
    // Leave some time for all pending communication to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    producer->ShutdownAndWait();
    producer.reset();

    fake_service->FinishAndDisallowRpc();
    fake_server->Shutdown();
    fake_server->Wait();

    fake_service.reset();
    fake_server.reset();
  }

  std::optional<FakeProducerSideService> fake_service;
  std::unique_ptr<grpc::Server> fake_server;
  std::optional<CaptureEventProducerImpl> producer;
};

constexpr std::chrono::duration kWaitMessagesSentDuration = std::chrono::milliseconds(25);

}  // namespace

TEST_F(CaptureEventProducerTest, OnCaptureStartStopAndIsCapturing) {
  EXPECT_FALSE(producer->IsCapturing());

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  fake_service->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  fake_service->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());
}

TEST_F(CaptureEventProducerTest, SendCaptureEventsAndAllEventsSent) {
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(2);
    EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(1);
  }

  orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest send_events_request;
  send_events_request.mutable_buffered_capture_events()->mutable_capture_events()->Add();
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->NotifyAllEventsSent());
}

TEST_F(CaptureEventProducerTest, UnexpectedStartStopCaptureCommands) {
  EXPECT_FALSE(producer->IsCapturing());

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStart).Times(0);
  // This should have no effect.
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  fake_service->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStop).Times(0);
  // This should have no effect.
  fake_service->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());
}

TEST_F(CaptureEventProducerTest, ServiceDisconnectCausesOnCaptureStop) {
  EXPECT_FALSE(producer->IsCapturing());

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  // Disconnect.
  fake_service->FinishAndDisallowRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());
}

TEST_F(CaptureEventProducerTest, SendingMessagesFailsWhenDisconnected) {
  EXPECT_FALSE(producer->IsCapturing());

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(2);
    EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(1);
  }
  orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest send_events_request;
  send_events_request.mutable_buffered_capture_events()->mutable_capture_events()->Add();
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->NotifyAllEventsSent());
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  // Disconnect.
  fake_service->FinishAndDisallowRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(producer->SendCaptureEvents(send_events_request));
  EXPECT_FALSE(producer->SendCaptureEvents(send_events_request));
  EXPECT_FALSE(producer->NotifyAllEventsSent());
}

TEST_F(CaptureEventProducerTest, DisconnectAndReconnect) {
  EXPECT_FALSE(producer->IsCapturing());

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(2);
    EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(1);
  }
  orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest send_events_request;
  send_events_request.mutable_buffered_capture_events()->mutable_capture_events()->Add();
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->NotifyAllEventsSent());
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service);

  // Reduce reconnection delay before disconnecting.
  static constexpr uint64_t kReconnectionDelayMs = 50;
  producer->SetReconnectionDelayMs(kReconnectionDelayMs);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  // Disconnect.
  fake_service->FinishAndDisallowRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(producer->SendCaptureEvents(send_events_request));
  EXPECT_FALSE(producer->SendCaptureEvents(send_events_request));
  EXPECT_FALSE(producer->NotifyAllEventsSent());
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service);

  // Wait for reconnection.
  fake_service->ReAllowRpc();
  std::this_thread::sleep_for(std::chrono::milliseconds{2 * kReconnectionDelayMs});

  EXPECT_CALL(*producer, OnCaptureStart).Times(1);
  fake_service->SendStartCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*producer);

  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_service, OnCaptureEventsReceived).Times(2);
    EXPECT_CALL(*fake_service, OnAllEventsSentReceived).Times(1);
  }
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->SendCaptureEvents(send_events_request));
  EXPECT_TRUE(producer->NotifyAllEventsSent());
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service);

  EXPECT_CALL(*producer, OnCaptureStop).Times(1);
  fake_service->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer->IsCapturing());
}

}  // namespace orbit_producer
