// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/types.h>

#include <memory>

#include "PerfEvent.h"
#include "PerfEventOrderedStream.h"
#include "PerfEventQueue.h"

namespace orbit_linux_tracing {

namespace {

// We do the testing with `ForkPerfEvent`s - that is just an arbitrary choice.
PerfEvent MakeTestEventNotOrdered(uint64_t timestamp) {
  return ForkPerfEvent{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::kNone,
  };
}

PerfEvent MakeTestEventOrderedInFd(int origin_fd, uint64_t timestamp) {
  return ForkPerfEvent{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(origin_fd),
  };
}

PerfEvent MakeTestEventOrderedInTid(pid_t tid, uint64_t timestamp) {
  return ForkPerfEvent{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::ThreadId(tid),
  };
}

}  // namespace

TEST(PerfEventQueue, SingleFd) {
  constexpr int kOriginFd = 11;
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp = 0;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventOrderedInFd(kOriginFd, 100));

  event_queue.PushEvent(MakeTestEventOrderedInFd(kOriginFd, 101));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  event_queue.PushEvent(MakeTestEventOrderedInFd(kOriginFd, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventOrderedInFd(kOriginFd, 103));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, FdWithDecreasingTimestamps) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 101));
  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 103));
  EXPECT_DEATH(event_queue.PushEvent(MakeTestEventOrderedInFd(11, 102)), "");
}

TEST(PerfEventQueue, TidWithDecreasingTimestamps) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 101));
  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 103));
  EXPECT_DEATH(event_queue.PushEvent(MakeTestEventOrderedInTid(11, 102)), "");
}

TEST(PerfEventQueue, MultipleFd) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 103));

  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 101));

  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  event_queue.PushEvent(MakeTestEventOrderedInFd(33, 100));

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 104));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 104;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, MultipleTids) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 103));

  event_queue.PushEvent(MakeTestEventOrderedInTid(22, 101));

  event_queue.PushEvent(MakeTestEventOrderedInTid(22, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  event_queue.PushEvent(MakeTestEventOrderedInTid(33, 100));

  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 104));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 104;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, FdWithOldestAndNewestEvent) {
  PerfEventQueue event_queue;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 101));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 102));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(33, 103));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(44, 104));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(55, 105));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(66, 106));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 999));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 102);
  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 103);
  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 104);
  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 105);
  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 106);
  event_queue.PopEvent();
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, 999);
  event_queue.PopEvent();
  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, NoOrder) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp = 0;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventNotOrdered(104));
  current_oldest_timestamp = 104;
  EXPECT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEventNotOrdered(101));
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEventNotOrdered(102));

  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();
  current_oldest_timestamp = 102;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();
  current_oldest_timestamp = 104;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  ASSERT_TRUE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEventNotOrdered(103));
  current_oldest_timestamp = 103;

  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();
  current_oldest_timestamp = 104;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();
  ASSERT_FALSE(event_queue.HasEvent());

  EXPECT_DEATH(event_queue.PopEvent(), "");
}

TEST(PerfEventQueue, OrderedInFdAndNoOrderTogether) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 103));
  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 105));
  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 102));
  event_queue.PushEvent(MakeTestEventNotOrdered(108));
  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 107));
  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 106));
  event_queue.PushEvent(MakeTestEventNotOrdered(101));
  event_queue.PushEvent(MakeTestEventNotOrdered(104));
  event_queue.PushEvent(MakeTestEventOrderedInFd(22, 109));

  uint64_t current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_FALSE(event_queue.HasEvent());
  EXPECT_DEATH(event_queue.PopEvent(), "");
}

TEST(PerfEventQueue, AllOrderTypesTogether) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 103));
  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 105));
  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 102));
  event_queue.PushEvent(MakeTestEventNotOrdered(108));
  event_queue.PushEvent(MakeTestEventOrderedInFd(11, 107));
  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 106));
  event_queue.PushEvent(MakeTestEventNotOrdered(101));
  event_queue.PushEvent(MakeTestEventNotOrdered(104));
  event_queue.PushEvent(MakeTestEventOrderedInTid(11, 109));

  uint64_t current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp++);
  event_queue.PopEvent();
  EXPECT_FALSE(event_queue.HasEvent());
  EXPECT_DEATH(event_queue.PopEvent(), "");
}

TEST(
    PerfEventQueue,
    TopEventAndPopEventReturnTheSameWhenAnEventOrderedByFdAndAnEventWithNoOrderHaveTheSameTimestamp) {
  PerfEventQueue event_queue;
  constexpr uint64_t kCommonTimestamp = 100;

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, kCommonTimestamp));
  event_queue.PushEvent(MakeTestEventNotOrdered(kCommonTimestamp));

  const uint64_t top_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream top_order = event_queue.TopEvent().ordered_stream;
  event_queue.PopEvent();

  const uint64_t remaining_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream remaining_order = event_queue.TopEvent().ordered_stream;

  EXPECT_EQ(top_timestamp, remaining_timestamp);
  EXPECT_NE(top_order, remaining_order);
}

TEST(
    PerfEventQueue,
    TopEventAndPopEventReturnTheSameWhenAnEventOrderedByTidAndAnEventWithNoOrderHaveTheSameTimestamp) {
  PerfEventQueue event_queue;
  constexpr uint64_t kCommonTimestamp = 100;

  event_queue.PushEvent(MakeTestEventOrderedInTid(11, kCommonTimestamp));
  event_queue.PushEvent(MakeTestEventNotOrdered(kCommonTimestamp));

  const uint64_t top_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream top_order = event_queue.TopEvent().ordered_stream;
  event_queue.PopEvent();

  const uint64_t remaining_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream remaining_order = event_queue.TopEvent().ordered_stream;

  EXPECT_EQ(top_timestamp, remaining_timestamp);
  EXPECT_NE(top_order, remaining_order);
}

TEST(
    PerfEventQueue,
    TopEventAndPopEventReturnTheSameWhenAnEventOrderedByFdAndAnEventOrderedByTidHaveTheSameTimestamp) {
  PerfEventQueue event_queue;
  constexpr uint64_t kCommonTimestamp = 100;

  event_queue.PushEvent(MakeTestEventOrderedInFd(11, kCommonTimestamp));
  event_queue.PushEvent(MakeTestEventOrderedInTid(22, kCommonTimestamp));

  const uint64_t top_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream top_order = event_queue.TopEvent().ordered_stream;
  event_queue.PopEvent();

  const uint64_t remaining_timestamp = event_queue.TopEvent().timestamp;
  const PerfEventOrderedStream remaining_order = event_queue.TopEvent().ordered_stream;

  EXPECT_EQ(top_timestamp, remaining_timestamp);
  EXPECT_NE(top_order, remaining_order);
}

}  // namespace orbit_linux_tracing
