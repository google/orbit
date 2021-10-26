// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <variant>

#include "PerfEvent.h"
#include "PerfEventQueue.h"

namespace orbit_linux_tracing {

namespace {

// We do the testing with ForkPerfEvent's - that is just an arbitrary choice.
PerfEvent MakeTestEvent(int origin_fd, uint64_t timestamp) {
  return ForkPerfEvent{
      .timestamp = timestamp,
      .ordered_in_file_descriptor = origin_fd,
  };
}

}  // namespace

TEST(PerfEventQueue, SingleFd) {
  constexpr int kOriginFd = 11;
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp = 0;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 100));

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 101));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 103));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, FdWithDecreasingTimestamps) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEvent(11, 101));
  event_queue.PushEvent(MakeTestEvent(11, 103));
  EXPECT_DEATH(event_queue.PushEvent(MakeTestEvent(11, 102)), "");
}

TEST(PerfEventQueue, MultipleFd) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(11, 103));

  event_queue.PushEvent(MakeTestEvent(22, 101));

  event_queue.PushEvent(MakeTestEvent(22, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);
  event_queue.PopEvent();

  event_queue.PushEvent(MakeTestEvent(33, 100));

  event_queue.PushEvent(MakeTestEvent(11, 104));

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

  event_queue.PushEvent(MakeTestEvent(11, 101));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(22, 102));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(33, 103));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(44, 104));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(55, 105));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(66, 106));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, 101);

  event_queue.PushEvent(MakeTestEvent(11, 999));
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

TEST(PerfEventQueue, NotOrderedInAnyFileDescriptor) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp = 0;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 104));
  current_oldest_timestamp = 104;
  EXPECT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 101));
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent().timestamp, current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 102));

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

  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 103));
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

TEST(PerfEventQueue, OrderedFdsAndNotOrderedInAnyFileDescriptor) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEvent(11, 103));
  event_queue.PushEvent(MakeTestEvent(11, 105));
  event_queue.PushEvent(MakeTestEvent(22, 102));
  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 108));
  event_queue.PushEvent(MakeTestEvent(11, 107));
  event_queue.PushEvent(MakeTestEvent(22, 106));
  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 101));
  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, 104));
  event_queue.PushEvent(MakeTestEvent(22, 109));

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
    TopEventAndPopEventReturnTheSameWhenAnEventOrderedByFdAndAnEventNotOrderedInAnyFdHaveTheSameTimestamp) {
  PerfEventQueue event_queue;
  constexpr uint64_t kCommonTimestamp = 100;

  event_queue.PushEvent(MakeTestEvent(11, kCommonTimestamp));
  event_queue.PushEvent(MakeTestEvent(kNotOrderedInAnyFileDescriptor, kCommonTimestamp));

  const uint64_t top_timestamp = event_queue.TopEvent().timestamp;
  const uint64_t top_fd = event_queue.TopEvent().ordered_in_file_descriptor;
  event_queue.PopEvent();

  const uint64_t remaining_timestamp = event_queue.TopEvent().timestamp;
  const uint64_t remaining_fd = event_queue.TopEvent().ordered_in_file_descriptor;

  EXPECT_EQ(top_timestamp, remaining_timestamp);
  EXPECT_NE(top_fd, remaining_fd);
}

}  // namespace orbit_linux_tracing
