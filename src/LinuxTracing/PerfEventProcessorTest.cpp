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

namespace orbit_linux_tracing {

namespace {

class MockVisitor : public PerfEventVisitor {
 public:
  MOCK_METHOD(void, visit, (LostPerfEvent * event), (override));
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

  static constexpr uint64_t kDelayBeforeProcessOldEventsMs = 100;
};

std::unique_ptr<PerfEvent> MakeFakePerfEvent(int origin_fd, uint64_t timestamp_ns) {
  // We use LostPerfEvent just because it's a simple one, but we could use any
  // as we only need to set the file descriptor and the timestamp.
  auto event = std::make_unique<LostPerfEvent>();
  event->SetOrderedInFileDescriptor(origin_fd);
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  return event;
}

}  // namespace

TEST_F(PerfEventProcessorTest, ProcessOldEvents) {
  EXPECT_CALL(mock_visitor_, visit).Times(0);
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(22, MonotonicTimestampNs()));
  processor_.ProcessOldEvents();

  ::testing::Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, visit).Times(3);
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);

  ::testing::Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, visit).Times(1);
  processor_.ProcessOldEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, ProcessAllEvents) {
  EXPECT_CALL(mock_visitor_, visit).Times(4);
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(22, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(33, MonotonicTimestampNs()));
  processor_.ProcessAllEvents();

  ::testing::Mock::VerifyAndClearExpectations(&mock_visitor_);

  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));

  EXPECT_CALL(mock_visitor_, visit).Times(0);
  processor_.ProcessAllEvents();
  EXPECT_EQ(discarded_out_of_order_counter_, 0);
}

TEST_F(PerfEventProcessorTest, DiscardedOutOfOrderCounter) {
  EXPECT_CALL(mock_visitor_, visit).Times(4);
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  uint64_t last_processed_timestamp_ns = MonotonicTimestampNs();
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

TEST_F(PerfEventProcessorTest, ProcessOldEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessOldEvents(), "!visitors_.empty()");
}

TEST_F(PerfEventProcessorTest, ProcessAllEventsNeedsVisitor) {
  processor_.ClearVisitors();
  processor_.AddEvent(MakeFakePerfEvent(11, MonotonicTimestampNs()));
  std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBeforeProcessOldEventsMs));
  EXPECT_DEATH(processor_.ProcessAllEvents(), "!visitors_.empty()");
}

}  // namespace orbit_linux_tracing
