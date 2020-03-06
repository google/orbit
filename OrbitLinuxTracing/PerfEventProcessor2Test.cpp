#include <gtest/gtest.h>

#include "PerfEventProcessor2.h"

namespace LinuxTracing {

namespace {
class TestEvent : public PerfEvent {
 public:
  explicit TestEvent(uint64_t timestamp) : timestamp_(timestamp) {}

  uint64_t GetTimestamp() const override { return timestamp_; }

  void Accept(PerfEventVisitor* visitor) override {}

 private:
  uint64_t timestamp_;
};

std::unique_ptr<PerfEvent> MakeTestEvent(uint64_t timestamp) {
  return std::make_unique<TestEvent>(timestamp);
}
}  // namespace

TEST(PerfEventQueue, SingleFd) {
  constexpr int origin_fd = 11;
  PerfEventQueue event_queue;
  uint64_t expected_timestamp;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(origin_fd, MakeTestEvent(100));

  event_queue.PushEvent(origin_fd, MakeTestEvent(101));

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  event_queue.PushEvent(origin_fd, MakeTestEvent(102));

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(origin_fd, MakeTestEvent(103));

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());
}

TEST(PerfEventQueue, MultipleFd) {
  PerfEventQueue event_queue;
  uint64_t expected_timestamp;

  EXPECT_FALSE(event_queue.HasEvent());

  event_queue.PushEvent(11, MakeTestEvent(103));

  event_queue.PushEvent(22, MakeTestEvent(101));

  event_queue.PushEvent(22, MakeTestEvent(102));

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 101;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 102;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  event_queue.PushEvent(33, MakeTestEvent(100));

  event_queue.PushEvent(11, MakeTestEvent(104));

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 100;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 103;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  ASSERT_TRUE(event_queue.HasEvent());
  expected_timestamp = 104;
  EXPECT_EQ(event_queue.TopEvent()->GetTimestamp(), expected_timestamp);
  EXPECT_EQ(event_queue.PopEvent()->GetTimestamp(), expected_timestamp);

  EXPECT_FALSE(event_queue.HasEvent());
}

}  // namespace LinuxTracing
