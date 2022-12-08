// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/channel_arguments.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "FakeProducerSideService/FakeProducerSideService.h"
#include "GrpcProtos/capture.pb.h"
#include "VulkanLayerProducer.h"
#include "VulkanLayerProducerImpl.h"

namespace orbit_vulkan_layer {

namespace {

class MockCaptureStatusListener : public VulkanLayerProducer::CaptureStatusListener {
 public:
  MOCK_METHOD(void, OnCaptureStart, (orbit_grpc_protos::CaptureOptions), (override));
  MOCK_METHOD(void, OnCaptureStop, (), (override));
  MOCK_METHOD(void, OnCaptureFinished, (), (override));
};

class VulkanLayerProducerImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fake_service_.emplace();

    grpc::ServerBuilder builder;
    builder.RegisterService(&fake_service_.value());
    fake_server_ = builder.BuildAndStart();
    ASSERT_NE(fake_server_, nullptr);

    std::shared_ptr<grpc::Channel> channel =
        fake_server_->InProcessChannel(grpc::ChannelArguments{});

    producer_.emplace();
    producer_->SetCaptureStatusListener(&mock_listener_);
    producer_->BringUp(channel);

    // Leave some time for the ReceiveCommandsAndSendEvents RPC to actually happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void TearDown() override {
    // Leave some time for all pending communication to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    producer_->TakeDown();
    producer_->SetCaptureStatusListener(nullptr);
    producer_.reset();

    fake_service_->FinishAndDisallowRpc();
    fake_server_->Shutdown();
    fake_server_->Wait();

    fake_service_.reset();
    fake_server_.reset();
  }

  std::optional<orbit_fake_producer_side_service::FakeProducerSideService> fake_service_;
  std::unique_ptr<grpc::Server> fake_server_;
  std::optional<VulkanLayerProducerImpl> producer_;
  MockCaptureStatusListener mock_listener_;

  static const std::string kInternedString1;
  static const uint64_t kExpectedInternedString1Key;
  static const std::string kInternedString2;
  static const uint64_t kExpectedInternedString2Key;
  static const std::string kInternedString3;
  static const uint64_t kExpectedInternedString3Key;
};

const std::string VulkanLayerProducerImplTest::kInternedString1 = "a";
const uint64_t VulkanLayerProducerImplTest::kExpectedInternedString1Key =
    std::hash<std::string>{}(kInternedString1);

const std::string VulkanLayerProducerImplTest::kInternedString2 = "b";
const uint64_t VulkanLayerProducerImplTest::kExpectedInternedString2Key =
    std::hash<std::string>{}(kInternedString2);

const std::string VulkanLayerProducerImplTest::kInternedString3 = "c";
const uint64_t VulkanLayerProducerImplTest::kExpectedInternedString3Key =
    std::hash<std::string>{}(kInternedString3);

constexpr std::chrono::milliseconds kWaitMessagesSentDuration{25};

const orbit_grpc_protos::CaptureOptions kFakeCaptureOptions = [] {
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(42);
  capture_options.set_samples_per_second(1234.0);
  return capture_options;
}();

MATCHER_P(CaptureOptionsEq, that, "") {
  const orbit_grpc_protos::CaptureOptions& a = arg;
  const orbit_grpc_protos::CaptureOptions& b = that;
  return a.SerializeAsString() == b.SerializeAsString();
}

TEST_F(VulkanLayerProducerImplTest, IsCapturingAndListener) {
  EXPECT_FALSE(producer_->IsCapturing());

  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);

  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());
}

TEST_F(VulkanLayerProducerImplTest, WorksWithNoListener) {
  EXPECT_CALL(mock_listener_, OnCaptureStart).Times(0);
  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(0);
  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(0);
  producer_->SetCaptureStatusListener(nullptr);

  EXPECT_FALSE(producer_->IsCapturing());

  fake_service_->SendStartCaptureCommand(orbit_grpc_protos::CaptureOptions{});
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_TRUE(producer_->IsCapturing());

  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_FALSE(producer_->IsCapturing());
}

TEST_F(VulkanLayerProducerImplTest, EnqueueCaptureEvent) {
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  std::atomic<uint64_t> capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        capture_events_received_count += events.size();
      });
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(::testing::Between(1, 3));
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_TRUE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
  EXPECT_TRUE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
  EXPECT_TRUE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  EXPECT_EQ(capture_events_received_count, 3);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  EXPECT_FALSE(producer_->EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent()));
}

void ExpectInternedStrings(absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> actual_events,
                           absl::Span<const std::pair<std::string, uint64_t> > expected_interns) {
  ASSERT_EQ(actual_events.size(), expected_interns.size());
  for (size_t i = 0; i < actual_events.size(); ++i) {
    ASSERT_EQ(actual_events[i].event_case(),
              orbit_grpc_protos::ProducerCaptureEvent::kInternedString);
    EXPECT_EQ(actual_events[i].interned_string().intern(), expected_interns[i].first);
    EXPECT_EQ(actual_events[i].interned_string().key(), expected_interns[i].second);
  }
}

TEST_F(VulkanLayerProducerImplTest, InternStringIfNecessaryAndGetKey) {
  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events_received;
  absl::Mutex events_received_mutex;
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived)
      .Times(::testing::Between(1, 2))
      .WillRepeatedly([&events_received, &events_received_mutex](
                          absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        absl::MutexLock lock{&events_received_mutex};
        events_received.insert(events_received.end(), events.begin(), events.end());
      });
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  uint64_t actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  {
    absl::MutexLock lock{&events_received_mutex};
    ExpectInternedStrings(events_received, {{kInternedString1, kExpectedInternedString1Key},
                                            {kInternedString2, kExpectedInternedString2Key}});
  }

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  // These should not be sent to the service.
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString3);
  EXPECT_EQ(actual_key, kExpectedInternedString3Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  // These should not be sent to the service.
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString3);
  EXPECT_EQ(actual_key, kExpectedInternedString3Key);
}

TEST_F(VulkanLayerProducerImplTest, DontSendInternTwice) {
  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events_received;
  absl::Mutex events_received_mutex;
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived)
      .Times(1)
      .WillRepeatedly([&events_received, &events_received_mutex](
                          absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        absl::MutexLock lock{&events_received_mutex};
        events_received.insert(events_received.end(), events.begin(), events.end());
      });
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  uint64_t actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  {
    absl::MutexLock lock{&events_received_mutex};
    ExpectInternedStrings(events_received, {{kInternedString1, kExpectedInternedString1Key}});
  }

  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
}

TEST_F(VulkanLayerProducerImplTest, ReInternInNewCapture) {
  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events_received;
  absl::Mutex events_received_mutex;
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived)
      .Times(::testing::Between(1, 2))
      .WillRepeatedly([&events_received, &events_received_mutex](
                          absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        absl::MutexLock lock{&events_received_mutex};
        events_received.insert(events_received.end(), events.begin(), events.end());
      });
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  uint64_t actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  {
    absl::MutexLock lock{&events_received_mutex};
    ExpectInternedStrings(events_received, {{kInternedString1, kExpectedInternedString1Key},
                                            {kInternedString2, kExpectedInternedString2Key}});
  }

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  {
    absl::MutexLock lock{&events_received_mutex};
    events_received.clear();
  }
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived)
      .Times(::testing::Between(1, 2))
      .WillRepeatedly([&events_received, &events_received_mutex](
                          absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        absl::MutexLock lock{&events_received_mutex};
        events_received.insert(events_received.end(), events.begin(), events.end());
      });
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  {
    absl::MutexLock lock{&events_received_mutex};
    ExpectInternedStrings(events_received, {{kInternedString1, kExpectedInternedString1Key},
                                            {kInternedString2, kExpectedInternedString2Key}});
  }

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
}

TEST_F(VulkanLayerProducerImplTest, InternOnlyWhenCapturing) {
  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  // These should not be sent to the service.
  uint64_t actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  // These should not be sent to the service.
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStart(CaptureOptionsEq(kFakeCaptureOptions))).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendStartCaptureCommand(kFakeCaptureOptions);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events_received;
  absl::Mutex events_received_mutex;
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived)
      .Times(::testing::Between(1, 2))
      .WillRepeatedly([&events_received, &events_received_mutex](
                          absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
        absl::MutexLock lock{&events_received_mutex};
        events_received.insert(events_received.end(), events.begin(), events.end());
      });
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString1);
  EXPECT_EQ(actual_key, kExpectedInternedString1Key);
  actual_key = producer_->InternStringIfNecessaryAndGetKey(kInternedString2);
  EXPECT_EQ(actual_key, kExpectedInternedString2Key);
  std::this_thread::sleep_for(kWaitMessagesSentDuration);
  {
    absl::MutexLock lock{&events_received_mutex};
    ExpectInternedStrings(events_received, {{kInternedString1, kExpectedInternedString1Key},
                                            {kInternedString2, kExpectedInternedString2Key}});
  }

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureStop).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(1);
  fake_service_->SendStopCaptureCommand();
  std::this_thread::sleep_for(kWaitMessagesSentDuration);

  ::testing::Mock::VerifyAndClearExpectations(&mock_listener_);
  ::testing::Mock::VerifyAndClearExpectations(&*fake_service_);

  EXPECT_CALL(mock_listener_, OnCaptureFinished).Times(1);
  EXPECT_CALL(*fake_service_, OnCaptureEventsReceived).Times(0);
  EXPECT_CALL(*fake_service_, OnAllEventsSentReceived).Times(0);
  fake_service_->SendCaptureFinishedCommand();
}

}  // namespace
}  // namespace orbit_vulkan_layer
