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
  EXPECT_THAT(timeline_ticks.GetMajorTicks(50, 250), testing::ElementsAre(100, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(50, 249), testing::ElementsAre(50, 100, 150, 200));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(1 * kSecondToNano, 6 * kSecondToNano),
              testing::ElementsAre(1 * kSecondToNano, 2 * kSecondToNano, 3 * kSecondToNano,
                                   4 * kSecondToNano, 5 * kSecondToNano, 6 * kSecondToNano));
  EXPECT_THAT(timeline_ticks.GetMajorTicks(40 * kSecondToNano, 1 * kMinuteToNano),
              testing::ElementsAre(40 * kSecondToNano, 50 * kSecondToNano, 1 * kMinuteToNano));
}

}  // namespace orbit_gl