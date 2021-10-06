// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "SimpleTimings.h"

namespace orbit_gl {
TEST(SimpleTimings, TestCalculations) {
  SimpleTimings timings(4);
  EXPECT_EQ(0, timings.GetAverageTimeInSeconds());
  EXPECT_EQ(0, timings.GetMinTimeInSeconds());
  EXPECT_EQ(0, timings.GetMaxTimeInSeconds());

  timings.PushTiming(10);
  EXPECT_EQ(10, timings.GetAverageTimeInSeconds());
  EXPECT_EQ(10, timings.GetMinTimeInSeconds());
  EXPECT_EQ(10, timings.GetMaxTimeInSeconds());

  timings.PushTiming(10);
  timings.PushTiming(0);
  timings.PushTiming(0);
  // Should average over 10 + 10 + 0 + 0
  EXPECT_EQ(5, timings.GetAverageTimeInSeconds());
  EXPECT_EQ(0, timings.GetMinTimeInSeconds());
  EXPECT_EQ(10, timings.GetMaxTimeInSeconds());

  timings.PushTiming(6);
  timings.PushTiming(6);
  // The two 10s should have been overwritten, so now we average over6 + 6 + 0 + 0
  EXPECT_EQ(3, timings.GetAverageTimeInSeconds());
  EXPECT_EQ(0, timings.GetMinTimeInSeconds());
  EXPECT_EQ(6, timings.GetMaxTimeInSeconds());

  timings.Reset();
  EXPECT_EQ(0, timings.GetAverageTimeInSeconds());
  EXPECT_EQ(0, timings.GetMinTimeInSeconds());
  EXPECT_EQ(0, timings.GetMaxTimeInSeconds());
}
}  // namespace orbit_gl