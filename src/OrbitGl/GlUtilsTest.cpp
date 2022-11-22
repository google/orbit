// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmath>
#include <memory>

#include "OrbitGl/GlUtils.h"

TEST(GlUtils, TicksToDuration) {
  uint64_t t0 = 0;
  uint64_t t1 = 1000;
  uint64_t t2 = 3000;
  EXPECT_TRUE(TicksToDuration(t0, t1) == absl::Microseconds(1));
  EXPECT_TRUE(TicksToDuration(t0, t2) == absl::Microseconds(3));
  EXPECT_TRUE(TicksToDuration(t1, t2) == absl::Microseconds(2));
}

TEST(GlUtils, TicksToMicroseconds) {
  uint64_t t0 = 0;
  uint64_t t1 = 1000;
  uint64_t t2 = 3000;
  double dt0 = TicksToMicroseconds(t0, t1);
  double dt1 = TicksToMicroseconds(t1, t2);
  EXPECT_TRUE(dt1 > dt0);
  constexpr double kEpsilon = 0.001;
  EXPECT_TRUE(abs(dt0 - 1.0) < kEpsilon);
  EXPECT_TRUE(abs(dt1 - 2.0) < kEpsilon);
}
