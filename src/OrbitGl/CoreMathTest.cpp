// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CoreMath.h"

namespace orbit_gl {

ClosedInterval kFakeClosedInterval{0, 3};

TEST(ClosedInterval, FromValues) {
  EXPECT_EQ(kFakeClosedInterval, ClosedInterval::FromValues(0, 3));
  EXPECT_EQ(kFakeClosedInterval, ClosedInterval::FromValues(3, 0));
}

TEST(ClosedInterval, Intersects) {
  EXPECT_TRUE((ClosedInterval{0, 3}.Intersects(ClosedInterval{1, 2})));
  EXPECT_TRUE((ClosedInterval{1, 2}.Intersects(ClosedInterval{0, 3})));
  EXPECT_TRUE((ClosedInterval{0, 1}.Intersects(ClosedInterval{1, 2})));
  EXPECT_TRUE((ClosedInterval{1, 2}.Intersects(ClosedInterval{0, 1})));
  EXPECT_FALSE((ClosedInterval{0, 1}.Intersects(ClosedInterval{2, 3})));
  EXPECT_FALSE((ClosedInterval{2, 3}.Intersects(ClosedInterval{0, 1})));
}

TEST(ClosedInterval, Contains) {
  EXPECT_FALSE(kFakeClosedInterval.Contains(-1));
  EXPECT_TRUE(kFakeClosedInterval.Contains(0));
  EXPECT_TRUE(kFakeClosedInterval.Contains(1));
  EXPECT_TRUE(kFakeClosedInterval.Contains(3));
  EXPECT_FALSE(kFakeClosedInterval.Contains(4));
}

TEST(CoreMath, IsInsideRectangle) {
  const Vec2 kFakeRectangleTopLeft{5, 5};
  const Vec2 kFakeRectangleSize{2, 2};
  EXPECT_FALSE(IsInsideRectangle({4, 4}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({4, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({6, 4}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({6, 8}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({8, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({8, 8}, kFakeRectangleTopLeft, kFakeRectangleSize));

  EXPECT_TRUE(IsInsideRectangle({5, 5}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({5, 7}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({6, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({7, 5}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({7, 7}, kFakeRectangleTopLeft, kFakeRectangleSize));
}

}  // namespace orbit_gl