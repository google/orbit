// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "SimpleTimings.h"

namespace orbit_gl {
TEST(SimpleTimings, TestCalculations) {
  SimpleTimings timings(4);
  EXPECT_EQ(0, timings.GetAverageTimeInMs());
  EXPECT_EQ(0, timings.GetMinTimeInMs());
  EXPECT_EQ(0, timings.GetMaxTimeInMs());

  timings.PushTimeMs(10);
  EXPECT_EQ(10, timings.GetAverageTimeInMs());
  EXPECT_EQ(10, timings.GetMinTimeInMs());
  EXPECT_EQ(10, timings.GetMaxTimeInMs());

  timings.PushTimeMs(10);
  timings.PushTimeMs(0);
  timings.PushTimeMs(0);
  // Should average over 10 + 10 + 0 + 0
  EXPECT_EQ(5, timings.GetAverageTimeInMs());
  EXPECT_EQ(0, timings.GetMinTimeInMs());
  EXPECT_EQ(10, timings.GetMaxTimeInMs());

  timings.PushTimeMs(6);
  timings.PushTimeMs(6);
  // The two 10s should have been overwritten, so now we average over6 + 6 + 0 + 0
  EXPECT_EQ(3, timings.GetAverageTimeInMs());
  EXPECT_EQ(0, timings.GetMinTimeInMs());
  EXPECT_EQ(6, timings.GetMaxTimeInMs());

  timings.Reset();
  EXPECT_EQ(0, timings.GetAverageTimeInMs());
  EXPECT_EQ(0, timings.GetMinTimeInMs());
  EXPECT_EQ(0, timings.GetMaxTimeInMs());
}
}  // namespace orbit_gl