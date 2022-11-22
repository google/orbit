// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

namespace {

class MockServerReaderWriter final
    : public grpc::ServerReaderWriterInterface<CaptureResponse, CaptureRequest> {
 public:
  void SendInitialMetadata() override {
    // This method should never be called.
    EXPECT_FALSE(true);
  }

  bool Write(const CaptureResponse& msg, grpc::WriteOptions options) override {
    EXPECT_EQ(options.flags(), grpc::WriteOptions{}.flags());
    EXPECT_EQ(options.is_last_message(), grpc::WriteOptions{}.is_last_message());
    OnCaptureResponse(msg);
    return true;
  }

  bool NextMessageSize(uint32_t* /*sz*/) override {
    // This method should never be called.
    EXPECT_FALSE(true);
    return false;
  }

  bool Read(CaptureRequest* /*msg*/) override {
    // This method should never be called.
    EXPECT_FALSE(true);
    return false;
  }

  MOCK_METHOD(void, OnCaptureResponse, (const CaptureResponse& capture_response), ());
};

class GrpcClientCaptureEventCollectorTest : public testing::Test {
 protected:
  void TearDown() override {
    if (!stop_and_wait_called_) {
      collector_.StopAndWait();
    }
  }

  void AddFakeEvents(uint64_t event_count) {
    for (uint64_t i = 0; i < event_count; ++i) {
      collector_.AddEvent(ClientCaptureEvent{});
    }
  }

  void CallStopAndWaitEarly() {
    ORBIT_CHECK(!stop_and_wait_called_);
    collector_.StopAndWait();
    stop_and_wait_called_ = true;
  }

  MockServerReaderWriter mock_reader_writer_{};

  // This should be higher than kSendTimeInterval in GrpcClientCaptureEventCollector::SenderThread.
  // We leave some margin to account for delays in scheduling.
  static constexpr std::chrono::milliseconds kWaitAllCaptureResponsesSentDuration{50};

 private:
  GrpcClientCaptureEventCollector collector_{&mock_reader_writer_};
  bool stop_and_wait_called_ = false;
};

}  // namespace

TEST_F(GrpcClientCaptureEventCollectorTest, AllEventsAreSent) {
  std::atomic<uint64_t> actual_event_count = 0;
  EXPECT_CALL(mock_reader_writer_, OnCaptureResponse)
      // While normally we expect a single CaptureResponse, the events could still be split across
      // two CaptureResponses.
      .Times(testing::Between(1, 2))
      .WillRepeatedly([&actual_event_count](const CaptureResponse& capture_response) {
        actual_event_count += capture_response.capture_events_size();
      });

  static constexpr uint64_t kEventCount = 5;
  AddFakeEvents(kEventCount);

  std::this_thread::sleep_for(kWaitAllCaptureResponsesSentDuration);
  EXPECT_EQ(actual_event_count, kEventCount);
}

TEST_F(GrpcClientCaptureEventCollectorTest, ManyEventsAreSplitAcrossMultipleCaptureResponses) {
  std::atomic<uint64_t> actual_event_count = 0;
  EXPECT_CALL(mock_reader_writer_, OnCaptureResponse)
      // This depends on the values of kSendEventCountInterval (5000) in
      // GrpcClientCaptureEventCollector::SenderThread, and of
      // kMaxEventsPerCaptureResponse (10000) in GrpcClientCaptureEventCollector::AddEvent.
      // So expect seven CaptureResponse, the first six of which with ~5000 events. But there could
      // be fewer CaptureResponses as they can fit up to 10000 events.
      .Times(testing::Between(4, 7))
      .WillRepeatedly([&actual_event_count](const CaptureResponse& capture_response) {
        actual_event_count += capture_response.capture_events_size();
      });

  static constexpr uint64_t kEventCount = 32000;
  AddFakeEvents(kEventCount);

  std::this_thread::sleep_for(kWaitAllCaptureResponsesSentDuration);
  EXPECT_EQ(actual_event_count, kEventCount);
}

TEST_F(GrpcClientCaptureEventCollectorTest, CaptureResponsesAreSentPeriodicallyEvenIfSmall) {
  std::atomic<uint64_t> actual_event_count = 0;
  EXPECT_CALL(mock_reader_writer_, OnCaptureResponse)
      .Times(2)
      .WillRepeatedly([&actual_event_count](const CaptureResponse& capture_response) {
        actual_event_count += capture_response.capture_events_size();
      });

  AddFakeEvents(1);
  std::this_thread::sleep_for(kWaitAllCaptureResponsesSentDuration);
  AddFakeEvents(1);

  std::this_thread::sleep_for(kWaitAllCaptureResponsesSentDuration);
  EXPECT_EQ(actual_event_count, 2);
}

TEST_F(GrpcClientCaptureEventCollectorTest, AllCaptureResponsesSentShortlyAfterStopAndWait) {
  std::atomic<uint64_t> actual_event_count = 0;
  EXPECT_CALL(mock_reader_writer_, OnCaptureResponse)
      .Times(testing::Between(1, 2))
      .WillRepeatedly([&actual_event_count](const CaptureResponse& capture_response) {
        actual_event_count += capture_response.capture_events_size();
      });

  static constexpr uint64_t kEventCount = 10;
  AddFakeEvents(kEventCount);

  CallStopAndWaitEarly();
  std::this_thread::sleep_for(std::chrono::milliseconds{1});
  EXPECT_EQ(actual_event_count, kEventCount);
}

}  // namespace orbit_producer_event_processor
