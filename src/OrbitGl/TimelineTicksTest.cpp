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

}  // namespace orbit_gl