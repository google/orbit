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
  void RunRpc(const std::shared_ptr<grpc::Channel>& channel) {
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
          case orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand:
            OnCaptureFinishedCommandReceived();
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
  MOCK_METHOD(void, OnCaptureFinishedCommandReceived, (), ());

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
    service_.emplace();

    grpc::ServerBuilder builder;
    builder.RegisterService(&*service_);
    fake_server_ = builder.BuildAndStart();

    std::shared_ptr<grpc::Channel> channel =
        fake_server_->InProcessChannel(grpc::ChannelArguments{});

    fake_producer_.emplace();
    fake_producer_->RunRpc(channel);

    // Leave some time for the ReceiveCommandsAndSendEvents RPC to actually happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void TearDown() override {
    // Leave some time for all pending communication to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    fake_producer_->FinishRpc();
    fake_producer_.reset();

    service_->OnExitRequest();
    fake_server_->Shutdown();
    fake_server_->Wait();

    service_.reset();
    fake_server_.reset();
  }

  std::optional<ProducerSideServiceImpl> service_;
  std::unique_ptr<grpc::Server> fake_server_;
  std::optional<FakeProducer> fake_producer_;
};

constexpr std::chrono::duration kWaitMessagesSentDuration = std::chrono::milliseconds(25);

void ExpectDurationBetweenMs(const std::function<void(void)>& action, uint64_t min_ms,
                             uint64_t max_ms) {
  auto begin = std::chrono::steady_clock::now();
  action();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(end - begin, std::chrono::milliseconds{min_ms});
  EXPECT_LE(end - begin, std::chrono::milliseconds{max_ms});
}

}  // namespace

TEST_F(ProducerSideServiceImplTest, OneCapture) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  static constexpr uint64_t kSendAllEventsDelayMs = 25;
  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds{kSendAllEventsDelayMs});
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, kSendAllEventsDelayMs,
                          2 * kSendAllEventsDelayMs);
}

TEST_F(ProducerSideServiceImplTest, TwoCaptures) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, 0,
                          kWaitMessagesSentDuration.count());
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(1);
  fake_producer_->SendBufferedCaptureEvents(2);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  static constexpr uint64_t kSendAllEventsDelayMs = 25;
  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds{kSendAllEventsDelayMs});
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, kSendAllEventsDelayMs,
                          2 * kSendAllEventsDelayMs);
}

TEST_F(ProducerSideServiceImplTest, NoCaptureEvents) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(0);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  static constexpr uint64_t kSendAllEventsDelayMs = 25;
  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds{kSendAllEventsDelayMs});
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, kSendAllEventsDelayMs,
                          2 * kSendAllEventsDelayMs);
}

TEST_F(ProducerSideServiceImplTest, NoAllEventsSent) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  static constexpr uint64_t kMaxWaitForAllCaptureEventsMs = 50;
  service_->SetMaxWaitForAllCaptureEvents(absl::Milliseconds(kMaxWaitForAllCaptureEventsMs));
  // As the AllEventsSent is not sent by the producer, OnCaptureStopRequested
  // this should take the time specified with SetMaxWaitForAllCaptureEvents.
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); },
                          kMaxWaitForAllCaptureEventsMs, 2 * kMaxWaitForAllCaptureEventsMs);
}

TEST_F(ProducerSideServiceImplTest, RedundantAllEventsSent) {
  MockCaptureEventBuffer mock_buffer;

  fake_producer_->SendAllEventsSent();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  static constexpr uint64_t kSendAllEventsDelayMs = 25;
  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds{kSendAllEventsDelayMs});
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, kSendAllEventsDelayMs,
                          2 * kSendAllEventsDelayMs);

  fake_producer_->SendAllEventsSent();
}

TEST_F(ProducerSideServiceImplTest, AllEventsSentBeforeStopCaptureCommand) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  fake_producer_->SendAllEventsSent();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  // As the producer has already sent AllEventsSent, this should be immediate.
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, 0, 5);
}

TEST_F(ProducerSideServiceImplTest, MultipleOnCaptureStartStop) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(0);
  // This should *not* cause StartCaptureCommand to be sent again.
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  static constexpr uint64_t kSendAllEventsDelayMs = 25;
  ON_CALL(*fake_producer_, OnStopCaptureCommandReceived).WillByDefault([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds{kSendAllEventsDelayMs});
    fake_producer_->SendAllEventsSent();
  });
  {
    ::testing::InSequence in_sequence;
    EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(1);
    EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(1);
  }
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, kSendAllEventsDelayMs,
                          2 * kSendAllEventsDelayMs);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(0);
  EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(0);
  // This should *not* cause StopCaptureCommand nor CaptureFinishedCommand to be sent again
  // and should be immediate.
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, 0,
                          kSendAllEventsDelayMs / 2);
}

TEST_F(ProducerSideServiceImplTest, NoOnCaptureStartRequested) {
  // As we are not waiting for any producer, this should be immediate.
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, 0, 5);
}

TEST_F(ProducerSideServiceImplTest, NoOnCaptureStopRequested) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(6);
  fake_producer_->SendBufferedCaptureEvents(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  EXPECT_CALL(*fake_producer_, OnStopCaptureCommandReceived).Times(0);
  EXPECT_CALL(*fake_producer_, OnCaptureFinishedCommandReceived).Times(0);
}

TEST_F(ProducerSideServiceImplTest, ProducerDisconnectsMidCapture) {
  MockCaptureEventBuffer mock_buffer;

  EXPECT_CALL(*fake_producer_, OnStartCaptureCommandReceived).Times(1);
  service_->OnCaptureStartRequested(&mock_buffer);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_producer_);

  EXPECT_CALL(mock_buffer, AddEvent).Times(3);
  fake_producer_->SendBufferedCaptureEvents(3);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  fake_producer_->FinishRpc();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_buffer);

  // As the producer has disconnected, this should be immediate.
  ExpectDurationBetweenMs([this] { service_->OnCaptureStopRequested(); }, 0, 5);
}

}  // namespace orbit_service
