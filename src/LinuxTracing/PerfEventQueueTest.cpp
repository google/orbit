// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>

#include "PerfEvent.h"
#include "PerfEventQueue.h"

namespace orbit_linux_tracing {

namespace {
class TestEvent : public PerfEvent {
 public:
  explicit TestEvent(int origin_fd, uint64_t timestamp) : timestamp_(timestamp) {
    SetOrderedInFileDescriptor(origin_fd);
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

TEST(PerfEventQueue, NotOrderedInAnyFileDescriptor) {
  PerfEventQueue event_queue;
  uint64_t current_oldest_timestamp = 0;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 104));
  current_oldest_timestamp = 104;
  EXPECT_TRUE(event_queue.HasEvent());
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 101));
  current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);

  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 102));

  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);
  current_oldest_timestamp = 102;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);
  current_oldest_timestamp = 104;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  ASSERT_TRUE(event_queue.HasEvent());

  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 103));
  current_oldest_timestamp = 103;

  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);
  current_oldest_timestamp = 104;
  ASSERT_TRUE(event_queue.HasEvent());

  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), current_oldest_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp);
  ASSERT_FALSE(event_queue.HasEvent());

  EXPECT_DEATH(event_queue.PopEvent(), "");
}

TEST(PerfEventQueue, OrderedFdsAndNotOrderedInAnyFileDescriptor) {
  PerfEventQueue event_queue;

  event_queue.PushEvent(MakeTestEvent(11, 103));
  event_queue.PushEvent(MakeTestEvent(11, 105));
  event_queue.PushEvent(MakeTestEvent(22, 102));
  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 108));
  event_queue.PushEvent(MakeTestEvent(11, 107));
  event_queue.PushEvent(MakeTestEvent(22, 106));
  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 101));
  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, 104));
  event_queue.PushEvent(MakeTestEvent(22, 109));

  uint64_t current_oldest_timestamp = 101;
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), current_oldest_timestamp++);
  EXPECT_FALSE(event_queue.HasEvent());
  EXPECT_DEATH(event_queue.PopEvent(), "");
}

TEST(
    PerfEventQueue,
    TopEventAndPopEventReturnTheSameWhenAnEventOrderedByFdAndAnEventNotOrderedInAnyFdHaveTheSameTimestamp) {
  PerfEventQueue event_queue;
  constexpr uint64_t kCommonTimestamp = 100;

  event_queue.PushEvent(MakeTestEvent(11, kCommonTimestamp));
  event_queue.PushEvent(MakeTestEvent(PerfEvent::kNotOrderedInAnyFileDescriptor, kCommonTimestamp));

  PerfEvent* top_event = event_queue.TopEvent();
  std::unique_ptr<PerfEvent> popped_event = event_queue.PopEvent();
  EXPECT_EQ(top_event, popped_event.get());
  EXPECT_EQ(popped_event->GetOrderedInFileDescriptor(), PerfEvent::kNotOrderedInAnyFileDescriptor);

  popped_event = event_queue.PopEvent();
  EXPECT_EQ(popped_event->GetOrderedInFileDescriptor(), 11);
}

}  // namespace orbit_linux_tracing
