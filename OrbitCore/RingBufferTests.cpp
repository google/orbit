#include "RingBuffer.h"

#include <cstdint>
#include <utility>

#include <gtest/gtest.h>

TEST(RingBuffer, Add) {
  RingBuffer<uint64_t, 5> ring_buffer;
  ring_buffer.Add(42);
  ring_buffer.Add(43);

  EXPECT_EQ(ring_buffer.Size(), 2);
  EXPECT_EQ(ring_buffer.Latest(), 43);
  EXPECT_EQ(ring_buffer[0], 42);
  ring_buffer.Add(44);
  EXPECT_EQ(ring_buffer[0], 42);
  EXPECT_EQ(ring_buffer.Size(), 3);
  ring_buffer.Add(45);
  EXPECT_EQ(ring_buffer[0], 42);
  EXPECT_EQ(ring_buffer.Size(), 4);
  ring_buffer.Add(46);
  EXPECT_EQ(ring_buffer[0], 42);
  EXPECT_EQ(ring_buffer.Size(), 5);
  ring_buffer.Add(47);
  EXPECT_EQ(ring_buffer[0], 43);
  EXPECT_EQ(ring_buffer.Size(), 5);
}

TEST(RingBuffer, Fill) {
  RingBuffer<uint64_t, 5> ring_buffer;
  ring_buffer.Add(42);
  EXPECT_EQ(ring_buffer[0], 42);
  ring_buffer.Fill(0);
  EXPECT_EQ(ring_buffer[0], 0);
}

TEST(RingBuffer, Contains) {
  RingBuffer<uint64_t, 5> ring_buffer;
  ring_buffer.Add(1);
  ring_buffer.Add(2);
  ring_buffer.Add(3);

  EXPECT_TRUE(ring_buffer.Contains(2));
  EXPECT_FALSE(ring_buffer.Contains(0));
}
