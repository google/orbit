// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <utility>

#include "LinuxTracing/TracerListener.h"
#include "LostAndDiscardedEventVisitor.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {

namespace {

class MockTracerListener : public TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
  MOCK_METHOD(void, OnCallstackSample, (orbit_grpc_protos::FullCallstackSample), (override));
  MOCK_METHOD(void, OnFunctionCall, (orbit_grpc_protos::FunctionCall), (override));
  MOCK_METHOD(void, OnIntrospectionScope, (orbit_grpc_protos::IntrospectionScope), (override));
  MOCK_METHOD(void, OnGpuJob, (orbit_grpc_protos::FullGpuJob full_gpu_job), (override));
  MOCK_METHOD(void, OnThreadName, (orbit_grpc_protos::ThreadName), (override));
  MOCK_METHOD(void, OnThreadNamesSnapshot, (orbit_grpc_protos::ThreadNamesSnapshot), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_grpc_protos::ThreadStateSlice), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_grpc_protos::FullAddressInfo), (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_grpc_protos::FullTracepointEvent), (override));
  MOCK_METHOD(void, OnModuleUpdate, (orbit_grpc_protos::ModuleUpdateEvent), (override));
  MOCK_METHOD(void, OnModulesSnapshot, (orbit_grpc_protos::ModulesSnapshot), (override));
  MOCK_METHOD(void, OnErrorsWithPerfEventOpenEvent,
              (orbit_grpc_protos::ErrorsWithPerfEventOpenEvent), (override));
  MOCK_METHOD(void, OnLostPerfRecordsEvent, (orbit_grpc_protos::LostPerfRecordsEvent), (override));
  MOCK_METHOD(void, OnOutOfOrderEventsDiscardedEvent,
              (orbit_grpc_protos::OutOfOrderEventsDiscardedEvent), (override));
};

[[nodiscard]] std::unique_ptr<LostPerfEvent> MakeFakeLostPerfEvent(uint64_t previous_timestamp_ns,
                                                                   uint64_t timestamp_ns) {
  auto event = std::make_unique<LostPerfEvent>();
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event->GetTimestamp() == timestamp_ns);
  event->SetPreviousTimestamp(previous_timestamp_ns);
  CHECK(event->GetPreviousTimestamp() == previous_timestamp_ns);
  return event;
}

[[nodiscard]] std::unique_ptr<DiscardedPerfEvent> MakeFakeDiscardedPerfEvent(
    uint64_t begin_timestamp_ns, uint64_t end_timestamp_ns) {
  auto event = std::make_unique<DiscardedPerfEvent>(begin_timestamp_ns, end_timestamp_ns);
  CHECK(event->GetTimestamp() == end_timestamp_ns);
  CHECK(event->GetBeginTimestampNs() == begin_timestamp_ns);
  CHECK(event->GetEndTimestampNs() == end_timestamp_ns);
  CHECK(event->GetTimestamp() == end_timestamp_ns);
  return event;
}

class LostAndDiscardedEventVisitorTest : public ::testing::Test {
 protected:
  void SetUp() override { visitor_.SetListener(&mock_listener_); }

  void TearDown() override {}

  LostAndDiscardedEventVisitor visitor_;
  MockTracerListener mock_listener_;
};

}  // namespace

TEST(LostAndDiscardedEventVisitor, NeedsVisitor) {
  LostAndDiscardedEventVisitor visitor;

  EXPECT_DEATH(MakeFakeLostPerfEvent(1111, 1234)->Accept(&visitor), "listener_ != nullptr");
  EXPECT_DEATH(MakeFakeDiscardedPerfEvent(1111, 1234)->Accept(&visitor), "listener_ != nullptr");
}

TEST_F(LostAndDiscardedEventVisitorTest, VisitLostPerfEventCallsOnLostPerfRecordsEvent) {
  orbit_grpc_protos::LostPerfRecordsEvent actual_lost_perf_records_event;
  EXPECT_CALL(mock_listener_, OnLostPerfRecordsEvent)
      .Times(1)
      .WillOnce(::testing::SaveArg<0>(&actual_lost_perf_records_event));

  constexpr uint64_t kPreviousTimestampNs = 1111;
  constexpr uint64_t kTimestampNs = 1234;
  MakeFakeLostPerfEvent(kPreviousTimestampNs, kTimestampNs)->Accept(&visitor_);

  EXPECT_EQ(actual_lost_perf_records_event.end_timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_lost_perf_records_event.duration_ns(), kTimestampNs - kPreviousTimestampNs);
}

TEST_F(LostAndDiscardedEventVisitorTest,
       VisitDiscardedPerfEventCallsOnOutOfOrderEventsDiscardedEvent) {
  orbit_grpc_protos::OutOfOrderEventsDiscardedEvent actual_out_of_order_events_discarded_event;
  EXPECT_CALL(mock_listener_, OnOutOfOrderEventsDiscardedEvent)
      .Times(1)
      .WillOnce(::testing::SaveArg<0>(&actual_out_of_order_events_discarded_event));

  constexpr uint64_t kBeginTimestampNs = 1111;
  constexpr uint64_t kEndTimestampNs = 1234;
  MakeFakeDiscardedPerfEvent(kBeginTimestampNs, kEndTimestampNs)->Accept(&visitor_);

  EXPECT_EQ(actual_out_of_order_events_discarded_event.end_timestamp_ns(), kEndTimestampNs);
  EXPECT_EQ(actual_out_of_order_events_discarded_event.duration_ns(),
            kEndTimestampNs - kBeginTimestampNs);
}

}  // namespace orbit_linux_tracing
