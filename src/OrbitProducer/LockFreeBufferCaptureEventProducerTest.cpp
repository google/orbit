// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/arena.h>
#include <grpcpp/server_impl.h>
#include <grpcpp/support/channel_arguments.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "Api/EncodedEvent.h"
#include "Api/LockFreeApiEventProducer.h"
#include "OrbitProducer/FakeProducerSideService.h"
#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "capture.pb.h"
#include "grpcpp/grpcpp.h"

namespace orbit_producer {

namespace {

class LockFreeBufferCaptureEventProducerImpl
    : public LockFreeBufferCaptureEventProducer<std::string> {
 protected:
  std::vector<orbit_grpc_protos::ProducerCaptureEvent*> TranslateIntermediateEvents(
      std::string* /*moveable_intermediate_events*/, size_t num_events,
      google::protobuf::Arena* arena) override {
    std::vector<orbit_grpc_protos::ProducerCaptureEvent*> capture_events(num_events);
    for (size_t i = 0; i < num_events; ++i) {
      capture_events[i] =
          google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
    }
    return capture_events;
  }
};

orbit_grpc_protos::ProducerCaptureEvent* CreateCaptureEventFixed(orbit_api::ApiEvent* event,
                                                                 google::protobuf::Arena* arena) {
  orbit_grpc_protos::ProducerCaptureEvent* capture_event =
      google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

  auto* api_event = capture_event->mutable_api_event_fixed();
  api_event->set_timestamp_ns(event->timestamp_ns);
  api_event->set_pid(event->pid);
  api_event->set_tid(event->tid);
  api_event->set_type(event->encoded_event.event.type);
  api_event->set_color(event->encoded_event.event.color);
  api_event->set_data(event->encoded_event.event.data);
  char* str = event->encoded_event.event.name;
  uint64_t* str_as_uint64 = reinterpret_cast<uint64_t*>(str);
  api_event->set_d0(str_as_uint64[0]);
  api_event->set_d1(str_as_uint64[1]);
  api_event->set_d2(str_as_uint64[2]);
  api_event->set_d3(str_as_uint64[3]);
  return capture_event;
}

orbit_grpc_protos::ProducerCaptureEvent* CreateCaptureEvent(orbit_api::ApiEvent* events,
                                                            size_t num_events,
                                                            google::protobuf::Arena* arena) {
  orbit_grpc_protos::ProducerCaptureEvent* capture_event =
      google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

  auto* api_event = capture_event->mutable_api_event();
  api_event->set_num_raw_events(num_events);
  api_event->mutable_raw_data()->Resize(num_events * sizeof(orbit_api::ApiEvent) / sizeof(uint64_t),
                                        0);
  void* buffer = api_event->mutable_raw_data()->mutable_data();
  std::memcpy(buffer, events, num_events * sizeof(orbit_api::ApiEvent));

  return capture_event;
}

TEST(ApiEvent, Performance) {
  size_t kNumApiEvents = 10000;
  std::vector<orbit_api::ApiEvent> api_events(kNumApiEvents);

  // Pre-allocate and always reuse the same 1 MB chunk of memory as the first block of each Arena
  // instance in the loop below. This is a small but measurable performance improvement.
  google::protobuf::ArenaOptions arena_options;
  constexpr size_t kArenaInitialBlockSize = 1024 * 1024;
  auto arena_initial_block = make_unique_for_overwrite<char[]>(kArenaInitialBlockSize);
  arena_options.initial_block = arena_initial_block.get();
  arena_options.initial_block_size = kArenaInitialBlockSize;
  google::protobuf::Arena arena_{arena_options};

  auto* arena = &arena_;
  // arena = nullptr;

  constexpr size_t kNumIterations = 100;

  for (size_t i = 0; i < kNumIterations; ++i) {
    LOG("iteration %u", i);
    // Create 10'000 capture events individually.
    {
      std::string msg = absl::StrFormat("Creating %u individual fixed events (arena=%p)",
                                        api_events.size(), arena);
      ScopeTimer t(msg);
      for (size_t i = 0; i < api_events.size(); ++i) {
        CreateCaptureEventFixed(&api_events[i], arena);
      }
    }

    // Create 10'000 capture events in bulk.
    {
      std::string msg =
          absl::StrFormat("Creating %u bulked api events (arena=%p)", api_events.size(), arena);
      ScopeTimer t(msg);
      CreateCaptureEvent(api_events.data(), api_events.size(), arena);
    }
  }

  ScopeTimer::OutputReport();
}

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

  std::optional<FakeProducerSideService> fake_service_;
  std::unique_ptr<grpc::Server> fake_server_;
  std::optional<LockFreeBufferCaptureEventProducerImpl> buffer_producer_;
};

constexpr std::chrono::duration kWaitMessagesSentDuration = std::chrono::milliseconds(25);

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

  int32_t capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         const std::vector<orbit_grpc_protos::ProducerCaptureEvent>& events) {
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

  int32_t capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         const std::vector<orbit_grpc_protos::ProducerCaptureEvent>& events) {
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

  int32_t capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         const std::vector<orbit_grpc_protos::ProducerCaptureEvent>& events) {
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

  int32_t capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         const std::vector<orbit_grpc_protos::ProducerCaptureEvent>& events) {
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

  int32_t capture_events_received_count = 0;
  ON_CALL(*fake_service_, OnCaptureEventsReceived)
      .WillByDefault([&capture_events_received_count](
                         const std::vector<orbit_grpc_protos::ProducerCaptureEvent>& events) {
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

}  // namespace orbit_producer
