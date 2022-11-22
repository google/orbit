// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include "OrbitBase/Profiling.h"
#include "PerfEvent.h"
#include "PerfEventOrderedStream.h"
#include "PerfEventProcessor.h"
#include "PerfEventVisitor.h"

using ::testing::_;
using ::testing::A;
using ::testing::Mock;

namespace orbit_linux_tracing {

namespace {

class MockVisitor : public PerfEventVisitor {
 public:
  MOCK_METHOD(void, Visit, (uint64_t event_timestamp, const ForkPerfEventData& event_data),
              (override));
  MOCK_METHOD(void, Visit, (uint64_t event_timestamp, const DiscardedPerfEventData& event_data),
              (override));
};

class PerfEventProcessorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    processor_.AddVisitor(&mock_visitor_);
    processor_.SetDiscardedOutOfOrderCounter(&discarded_out_of_order_counter_);
  }

  void TearDown() override {}

  PerfEventProcessor processor_;
  MockVisitor mock_visitor_;
  std::atomic<uint64_t> discarded_out_of_order_counter_ = 0;

  static constexpr uint64_t kDelayBeforeProcessOldEventsMs = PerfEventProcessor::kProcessingDelayMs;
};

PerfEvent MakeFakePerfEventOrderedInFd(int origin_fd, uint64_t timestamp_ns) {
  // We use ForkPerfEvent just because it's a simple one, but we could use any
  // as we only need to set the file descriptor and the timestamp.
  return ForkPerfEvent{
      .timestamp = timestamp_ns,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(origin_fd),
  };
}

PerfEvent MakeFakePerfEventNotOrdered(uint64_t timestamp_ns) {
  return ForkPerfEvent{
      .timestamp = timestamp_ns,
      .ordered_stream = PerfEventOrderedStream::kNone,
  };
}

MATCHER_P(UntypedDiscardedPerfEventDataEq, begin_timestamp_ns, "") {
  const DiscardedPerfEventData& event_data = arg;
  return event_data.begin_timestamp_ns == begin_timestamp_ns;
}

auto DiscardedPerfEventDataEq(uint64_t begin_timestamp_ns) {
  return ::testing::Matcher<const DiscardedPerfEventData&>(
      UntypedDiscardedPerfEventDataEq<uint64_t>(begin_timestamp_ns));
}

}  // namespace

TEST_F(PerfEventProcessorTest, ProcessOldEvents) {
  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(0);
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(22, orbit_base::CaptureTimestampNs()));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(3);
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(1);
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, ProcessAllEvents) {
  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(4);
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(22, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(33, orbit_base::CaptureTimestampNs()));
  processor_.ProcessAllEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(0);
  processor_.ProcessAllEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, DiscardedOutOfOrderCounter) {
  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(4);
  EXPECT_CALL(mock_visitor_, Visit(_, A<const DiscardedPerfEventData&>())).Times(1);

  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  uint64_t last_processed_timestamp_ns = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(22, last_processed_timestamp_ns));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, last_processed_timestamp_ns));
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, last_processed_timestamp_ns - 1));
  EXPECT_EQ(discarded_out_of_order_counter_, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();
}

TEST_F(PerfEventProcessorTest, DiscardedPerfEvents) {
  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(1);
  EXPECT_CALL(mock_visitor_, Visit(_, A<const DiscardedPerfEventData&>())).Times(0);

  uint64_t last_processed_timestamp_ns1 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns1));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // First discarded range.
  EXPECT_CALL(mock_visitor_, Visit(last_processed_timestamp_ns1,
                                   DiscardedPerfEventDataEq(last_processed_timestamp_ns1 - 10)))
      .Times(1);
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns1 - 10));

  // Discarded range ends at the same timestamp as the previous one, but starts earlier causing a
  // new DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(last_processed_timestamp_ns1,
                                   DiscardedPerfEventDataEq(last_processed_timestamp_ns1 - 15)))
      .Times(1);
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns1 - 15));

  // Discarded range ends at the same timestamp as the previous one, and starts later, so no new
  // DiscardedPerfEvent is generated.
  EXPECT_CALL(mock_visitor_, Visit(last_processed_timestamp_ns1,
                                   DiscardedPerfEventDataEq(last_processed_timestamp_ns1 - 5)))
      .Times(0);
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns1 - 5));

  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(1);
  const uint64_t last_processed_timestamp_ns2 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns2));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // Discarded range starts in the previous discarded range, but ends after it, causing a new
  // DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(last_processed_timestamp_ns2,
                                   DiscardedPerfEventDataEq(last_processed_timestamp_ns1 - 5)))
      .Times(1);
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns1 - 5));

  EXPECT_CALL(mock_visitor_, Visit(_, A<const ForkPerfEventData&>())).Times(1);
  uint64_t last_processed_timestamp_ns3 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns3));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // Discarded range starts and ends after the previous one, causing a new DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(last_processed_timestamp_ns3,
                                   DiscardedPerfEventDataEq(last_processed_timestamp_ns2 + 10)))
      .Times(1);
  processor_.AddEvent(MakeFakePerfEventNotOrdered(last_processed_timestamp_ns2 + 10));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  EXPECT_EQ(discarded_out_of_order_counter_, 5);
}

TEST_F(PerfEventProcessorTest, ProcessOldEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessOldEvents(), "!visitors_.empty()");
}

TEST_F(PerfEventProcessorTest, ProcessAllEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEventOrderedInFd(11, orbit_base::CaptureTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessAllEvents(), "!visitors_.empty()");
}

}  // namespace orbit_linux_tracing
