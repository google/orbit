// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "OrbitBase/Profiling.h"
#include "PerfEvent.h"
#include "PerfEventProcessor.h"
#include "PerfEventRecords.h"
#include "PerfEventVisitor.h"

using ::testing::A;
using ::testing::Mock;

namespace orbit_linux_tracing {

namespace {

class MockVisitor : public PerfEventVisitor {
 public:
  MOCK_METHOD(void, Visit, (ForkPerfEvent * event), (override));
  MOCK_METHOD(void, Visit, (DiscardedPerfEvent * event), (override));
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

PerfEvent MakeFakePerfEvent(int origin_fd, uint64_t timestamp_ns) {
  // We use ForkPerfEvent just because it's a simple one, but we could use any
  // as we only need to set the file descriptor and the timestamp.
  return ForkPerfEvent{.ordered_in_file_descriptor = origin_fd, .timestamp = timestamp_ns};
}

MATCHER_P2(UntypedDiscardedPerfEventEq, begin_timestamp_ns, end_timestamp_ns, "") {
  const DiscardedPerfEvent& event = *arg;
  return event.begin_timestamp_ns == begin_timestamp_ns && event.timestamp == end_timestamp_ns;
}

auto DiscardedPerfEventEq(uint64_t begin_timestamp_ns, uint64_t end_timestamp_ns) {
  return ::testing::Matcher<DiscardedPerfEvent*>(
      UntypedDiscardedPerfEventEq<uint64_t, uint64_t>(begin_timestamp_ns, end_timestamp_ns));
}

}  // namespace

TEST_F(PerfEventProcessorTest, ProcessOldEvents) {
  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(0);
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(22, orbit_base::CaptureTimestampNs()));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(3);
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(1);
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, ProcessAllEvents) {
  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(4);
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(22, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(33, orbit_base::CaptureTimestampNs()));
  processor_.ProcessAllEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(0);
  processor_.ProcessAllEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, DiscardedOutOfOrderCounter) {
  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(4);
  EXPECT_CALL(mock_visitor_, Visit(A<DiscardedPerfEvent*>())).Times(1);

  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  uint64_t last_processed_timestamp_ns = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(MakeFakePerfEvent(22, last_processed_timestamp_ns));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  processor_.AddEvent(MakeFakePerfEvent(11, last_processed_timestamp_ns));
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
  processor_.AddEvent(MakeFakePerfEvent(11, last_processed_timestamp_ns - 1));
  EXPECT_EQ(discarded_out_of_order_counter_, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();
}

TEST_F(PerfEventProcessorTest, DiscardedPerfEvents) {
  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(1);
  EXPECT_CALL(mock_visitor_, Visit(A<DiscardedPerfEvent*>())).Times(0);

  uint64_t last_processed_timestamp_ns1 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns1));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // First discarded range.
  EXPECT_CALL(mock_visitor_, Visit(DiscardedPerfEventEq(last_processed_timestamp_ns1 - 10,
                                                        last_processed_timestamp_ns1)))
      .Times(1);
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns1 - 10));

  // Discarded range ends at the same timestamp as the previous one, but starts earlier causing a
  // new DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(DiscardedPerfEventEq(last_processed_timestamp_ns1 - 15,
                                                        last_processed_timestamp_ns1)))
      .Times(1);
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns1 - 15));

  // Discarded range ends at the same timestamp as the previous one, and starts later, so no new
  // DiscardedPerfEvent is generated.
  EXPECT_CALL(mock_visitor_, Visit(DiscardedPerfEventEq(last_processed_timestamp_ns1 - 5,
                                                        last_processed_timestamp_ns1)))
      .Times(0);
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns1 - 5));

  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(1);
  const uint64_t last_processed_timestamp_ns2 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns2));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // Discarded range starts in the previous discarded range, but ends after it, causing a new
  // DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(DiscardedPerfEventEq(last_processed_timestamp_ns1 - 5,
                                                        last_processed_timestamp_ns2)))
      .Times(1);
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns1 - 5));

  EXPECT_CALL(mock_visitor_, Visit(A<ForkPerfEvent*>())).Times(1);
  uint64_t last_processed_timestamp_ns3 = orbit_base::CaptureTimestampNs();
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns3));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  Mock::VerifyAndClearExpectations(&mock_visitor_);

  // Discarded range starts and ends after the previous one, causing a new DiscardedPerfEvent.
  EXPECT_CALL(mock_visitor_, Visit(DiscardedPerfEventEq(last_processed_timestamp_ns2 + 10,
                                                        last_processed_timestamp_ns3)))
      .Times(1);
  processor_.AddEvent(
      MakeFakePerfEvent(kNotOrderedInAnyFileDescriptor, last_processed_timestamp_ns2 + 10));

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  processor_.ProcessOldEvents();

  EXPECT_EQ(discarded_out_of_order_counter_, 5);
}

TEST_F(PerfEventProcessorTest, ProcessOldEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessOldEvents(), "!visitors_.empty()");
}

TEST_F(PerfEventProcessorTest, ProcessAllEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEvent(11, orbit_base::CaptureTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessAllEvents(), "!visitors_.empty()");
}

}  // namespace orbit_linux_tracing
