// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "PerfEventQueue.h"

namespace LinuxTracing {

namespace {
class TestEvent : public PerfEvent {
 public:
  explicit TestEvent(int origin_fd, uint64_t timestamp) : timestamp_(timestamp) {
    SetOriginFileDescriptor(origin_fd);
  }

  uint64_t GetTimestamp() const override { return timestamp_; }

  void Accept(PerfEventVisitor* /*visitor*/) override {}

 private:
  uint64_t timestamp_;
};

std::unique_ptr<PerfEvent> MakeTestEvent(int origin_fd, uint64_t timestamp) {
  return std::make_unique<TestEvent>(origin_fd, timestamp);
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
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 102));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(kOriginFd, 103));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());
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
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(33, 100));

  event_queue.PushEvent(MakeTestEvent(11, 104));

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  current_oldest_timestamp = 104;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, FdWithOldestAndNewestEvent) {
  PerfEventQueue event_queue;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(11, 101));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(22, 102));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(33, 103));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(44, 104));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(55, 105));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(66, 106));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  event_queue.PushEvent(MakeTestEvent(11, 999));
  ASSERT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), 101);

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 101);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 102);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 103);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 104);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 105);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 106);
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), 999);
  EXPECT_FALSE(event_queue.HasEvent());
}

}  // namespace LinuxTracing
