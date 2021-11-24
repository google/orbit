// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>

#include "GrpcProtos/capture.pb.h"
#include "LostAndDiscardedEventVisitor.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"

namespace orbit_linux_tracing {

namespace {

[[nodiscard]] LostPerfEvent MakeFakeLostPerfEvent(uint64_t previous_timestamp_ns,
                                                  uint64_t timestamp_ns) {
  return LostPerfEvent{
      .timestamp = timestamp_ns,
      .data =
          {
              .previous_timestamp = previous_timestamp_ns,
          },
  };
}

[[nodiscard]] DiscardedPerfEvent MakeFakeDiscardedPerfEvent(uint64_t begin_timestamp_ns,
                                                            uint64_t end_timestamp_ns) {
  return DiscardedPerfEvent{
      .timestamp = end_timestamp_ns,
      .data =
          {
              .begin_timestamp_ns = begin_timestamp_ns,
          },
  };
}

class LostAndDiscardedEventVisitorTest : public ::testing::Test {
 protected:
  MockTracerListener mock_listener_;
  LostAndDiscardedEventVisitor visitor_{&mock_listener_};
};

}  // namespace

TEST(LostAndDiscardedEventVisitor, NeedsVisitor) {
  EXPECT_DEATH(LostAndDiscardedEventVisitor{nullptr}, "listener_ != nullptr");
}

TEST_F(LostAndDiscardedEventVisitorTest, VisitLostPerfEventCallsOnLostPerfRecordsEvent) {
  orbit_grpc_protos::LostPerfRecordsEvent actual_lost_perf_records_event;
  EXPECT_CALL(mock_listener_, OnLostPerfRecordsEvent)
      .Times(1)
      .WillOnce(::testing::SaveArg<0>(&actual_lost_perf_records_event));

  constexpr uint64_t kPreviousTimestampNs = 1111;
  constexpr uint64_t kTimestampNs = 1234;
  PerfEvent{MakeFakeLostPerfEvent(kPreviousTimestampNs, kTimestampNs)}.Accept(&visitor_);

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
  PerfEvent{MakeFakeDiscardedPerfEvent(kBeginTimestampNs, kEndTimestampNs)}.Accept(&visitor_);

  EXPECT_EQ(actual_out_of_order_events_discarded_event.end_timestamp_ns(), kEndTimestampNs);
  EXPECT_EQ(actual_out_of_order_events_discarded_event.duration_ns(),
            kEndTimestampNs - kBeginTimestampNs);
}

}  // namespace orbit_linux_tracing
