// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TimelineTicks.h"

namespace orbit_gl {

TEST(TimelineTicks, GetMajorTicks) {
  TimelineTicks timeline_ticks;

  EXPECT_THAT(timeline_ticks.GetMajorTicks(0, 20), testing::ElementsAre(0, 10, 20));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(0, 299), testing::ElementsAre(0, 100, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(1, 299), testing::ElementsAre(100, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(50, 249), testing::ElementsAre(100, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(50, 248), testing::ElementsAre(50, 100, 150, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(1 * kNanosecondsPerSecond, 6 * kNanosecondsPerSecond),
              testing::ElementsAre(1 * kNanosecondsPerSecond, 2 * kNanosecondsPerSecond,
                                   3 * kNanosecondsPerSecond, 4 * kNanosecondsPerSecond,
                                   5 * kNanosecondsPerSecond, 6 * kNanosecondsPerSecond));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(40 * kNanosecondsPerSecond, 1 * kNanosecondsPerMinute),
              testing::ElementsAre(40 * kNanosecondsPerSecond, 50 * kNanosecondsPerSecond,
                                   1 * kNanosecondsPerMinute));
}

bool CheckTicks(uint64_t start_ns, uint64_t end_ns, std::set<uint64_t> major_ticks,
                std::set<uint64_t> minor_ticks) {
  TimelineTicks timeline_ticks;
  auto all_ticks = timeline_ticks.GetAllTicks(start_ns, end_ns);
  if (all_ticks.size() != major_ticks.size() + minor_ticks.size()) return false;
  for (auto& [tick_type, timestamp_ns] : all_ticks) {
    if (tick_type == TimelineTicks::TickType::kMajorTick && major_ticks.count(timestamp_ns) == 0) {
      return false;
    }
    if (tick_type == TimelineTicks::TickType::kMinorTick && minor_ticks.count(timestamp_ns) == 0) {
      return false;
    }
  }
  return true;
}

TEST(TimelineTicks, GetAllTicks) {
  CheckTicks(0, 20, {0, 10, 20}, {5, 15});
  CheckTicks(0, 299, {0, 100, 200}, {50, 150});
  CheckTicks(1, 299, {100, 200}, {50, 150});
  CheckTicks(50, 248, {50, 100, 150, 200},
             {60, 70, 80, 90, 110, 120, 130, 140, 160, 170, 180, 190, 210, 220, 230, 240});
  CheckTicks(40 * kNanosecondsPerSecond, 1 * kNanosecondsPerMinute,
             {40 * kNanosecondsPerSecond, 50 * kNanosecondsPerSecond, 1 * kNanosecondsPerMinute},
             {45 * kNanosecondsPerSecond, 55 * kNanosecondsPerSecond});
}

TEST(TimelineTicks, GetTimestampNumDigitsPrecision) {
  TimelineTicks timeline_ticks;

  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(10), 8);   // 10ns = 0.000'000'01s
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(200), 7);  // 200ns = 0.000'000'2s
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(297), 9);
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(40 * kNanosecondsPerMicrosecond),
            5);  // 0.040 ms = 0.000'04s
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(1 * kNanosecondsPerSecond), 0);
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(1 * kNanosecondsPerMinute), 0);
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(3 * kNanosecondsPerHour +
                                                          2 * kNanosecondsPerSecond),
            0);
  EXPECT_EQ(timeline_ticks.GetTimestampNumDigitsPrecision(3 * kNanosecondsPerHour + 10), 8);
}

}  // namespace orbit_gl