// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

#include "ProducerSideServiceImpl.h"
#include "absl/strings/str_format.h"
#include "grpcpp/grpcpp.h"

namespace orbit_service {

namespace {

// This class fakes a client of ProducerSideService for use in tests.
class FakeProducer {
 public:
  void RunRpc(std::string_view unix_domain_socket_path) {
    std::string server_address = absl::StrFormat("unix:%s", unix_domain_socket_path);
    std::shared_ptr<grpc::Channel> channel = grpc::CreateCustomChannel(
        server_address, grpc::InsecureChannelCredentials(), grpc::ChannelArguments{});
    ASSERT_NE(channel, nullptr);

    std::unique_ptr<orbit_grpc_protos::ProducerSideService::Stub> stub =
        orbit_grpc_protos::ProducerSideService::NewStub(channel);
    ASSERT_NE(stub, nullptr);

    EXPECT_EQ(context_, nullptr);
    EXPECT_EQ(stream_, nullptr);
    context_ = std::make_unique<grpc::ClientContext>();
    stream_ = stub->ReceiveCommandsAndSendEvents(context_.get());
    ASSERT_NE(stream_, nullptr);

    read_thread_ = std::thread([this] {
      orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse response;
      while (stream_->Read(&response)) {
        EXPECT_NE(response.command_case(),
                  orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::COMMAND_NOT_SET);
        switch (response.command_case()) {
          case orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand:
            OnStartCaptureCommandReceived();
            break;
          case orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand:
            OnStopCaptureCommandReceived();
            break;
          case orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::COMMAND_NOT_SET:
            break;
        }
      }
    });
  }

  void SendBufferedCaptureEvents(int32_t num_to_send) {
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest request;
    request.mutable_buffered_capture_events();
    for (int32_t i = 0; i < num_to_send; ++i) {
      request.mutable_buffered_capture_events()->mutable_capture_events()->Add();
    }
    bool written = stream_->Write(request);
    EXPECT_TRUE(written);
  }

  void SendAllEventsSent() {
    ASSERT_NE(stream_, nullptr);
    orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest request;
    request.mutable_all_events_sent();
    bool written = stream_->Write(request);
    EXPECT_TRUE(written);
  }

  void FinishRpc() {
    if (context_ != nullptr) {
      context_->TryCancel();
      context_ = nullptr;
      EXPECT_NE(stream_, nullptr);
      EXPECT_TRUE(read_thread_.joinable());
    }
    stream_ = nullptr;
    if (read_thread_.joinable()) {
      read_thread_.join();
    }
  }

  MOCK_METHOD(void, OnStartCaptureCommandReceived, (), ());
  MOCK_METHOD(void, OnStopCaptureCommandReceived, (), ());

 private:
  std::unique_ptr<grpc::ClientContext> context_;
  std::unique_ptr<grpc::ClientReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest,
                                           orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse>>
      stream_;
  std::thread read_thread_;
};

class MockCaptureEventBuffer : public CaptureEventBuffer {
 public:
  MOCK_METHOD(void, AddEvent, (orbit_grpc_protos::CaptureEvent && event), (override));
};

class ProducerSideServiceImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    static const std::string kUnixDomainSocketPath = "./producer-side-service-impl-test-socket";

    grpc::ServerBuilder builder;
    builder.AddListeningPort(absl::StrFormat("unix:%s", kUnixDomainSocketPath),
                             grpc::InsecureServerCredentials());

    service.emplace();
    builder.RegisterService(&*service);
    fake_server = builder.BuildAndStart();

    fake_producer.emplace();
    ON_CALL(*fake_producer, OnStopCaptureCommandReceived).WillByDefault([this] {
      fake_producer->SendAllEventsSent();
    });
    fake_producer->RunRpc(kUnixDomainSocketPath);

    // Leave some time for the ReceiveCommandsAndSendEvents RPC to actually happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void TearDown() override {
    // Leave some time for all pending communication to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    fake_producer->FinishRpc();
    fake_producer.reset();

    service->OnExitRequest();
    fake_server->Shutdown();
    fake_server->Wait();

    service.reset();
    fake_server.reset();
  }

  std::optional<ProducerSideServiceImpl> service;
  std::unique_ptr<grpc::Server> fake_server;
  std::optional<FakeProducer> fake_producer;
};

constexpr std::chrono::duration kWaitMessagesSentDuration = std::chrono::milliseconds(25);

}  // namespace

TEST_F(ProducerSideServiceImplTest, OneCapture) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer, OnStartCaptureCommandReceived).Times(1);
  service->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer->SendBufferedCaptureEvents(3);
  fake_producer->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  EXPECT_CALL(*fake_producer, OnStopCaptureCommandReceived).Times(1);
  service->OnCaptureStopRequested();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
}

TEST_F(ProducerSideServiceImplTest, TwoCaptures) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer, OnStartCaptureCommandReceived).Times(1);
  service->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer->SendBufferedCaptureEvents(3);
  fake_producer->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  EXPECT_CALL(*fake_producer, OnStopCaptureCommandReceived).Times(1);
  service->OnCaptureStopRequested();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(*fake_producer, OnStartCaptureCommandReceived).Times(1);
  service->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer->SendBufferedCaptureEvents(1);
  fake_producer->SendBufferedCaptureEvents(2);
  fake_producer->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  EXPECT_CALL(*fake_producer, OnStopCaptureCommandReceived).Times(1);
  service->OnCaptureStopRequested();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
}

TEST_F(ProducerSideServiceImplTest, MultipleOnCaptureStartStop) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer, OnStartCaptureCommandReceived).Times(1);
  service->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(*fake_producer, OnStartCaptureCommandReceived).Times(0);
  // This should *not* cause StartCaptureCommand to be sent again.
  service->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer->SendBufferedCaptureEvents(3);
  fake_producer->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  EXPECT_CALL(*fake_producer, OnStopCaptureCommandReceived).Times(1);
  service->OnCaptureStopRequested();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer);

  EXPECT_CALL(*fake_producer, OnStopCaptureCommandReceived).Times(0);
  // This should *not* cause StopCaptureCommand to be sent again.
  service->OnCaptureStopRequested();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
}

}  // namespace orbit_service
