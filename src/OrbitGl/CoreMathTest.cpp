// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CoreMath.h"

namespace orbit_gl {

const ClosedInterval<int> kSmallClosedInterval{0, 3};

TEST(ClosedInterval, Intersects) {
  EXPECT_TRUE((ClosedInterval<int>{0, 3}.Intersects(ClosedInterval<int>{1, 2})));
  EXPECT_TRUE((ClosedInterval<int>{1, 2}.Intersects(ClosedInterval<int>{0, 3})));
  EXPECT_TRUE((ClosedInterval<int>{0, 1}.Intersects(ClosedInterval<int>{1, 2})));
  EXPECT_TRUE((ClosedInterval<int>{1, 2}.Intersects(ClosedInterval<int>{0, 1})));
  EXPECT_FALSE((ClosedInterval<int>{0, 1}.Intersects(ClosedInterval<int>{2, 3})));
  EXPECT_FALSE((ClosedInterval<int>{2, 3}.Intersects(ClosedInterval<int>{0, 1})));

  EXPECT_TRUE(kSmallClosedInterval.Intersects(ClosedInterval<int>{1, 1}));
  EXPECT_TRUE(kSmallClosedInterval.Intersects(ClosedInterval<int>{3, 3}));
  EXPECT_FALSE(kSmallClosedInterval.Intersects(ClosedInterval<int>{4, 4}));
}

TEST(ClosedInterval, Contains) {
  EXPECT_FALSE(kSmallClosedInterval.Contains(-1));
  EXPECT_TRUE(kSmallClosedInterval.Contains(0));
  EXPECT_TRUE(kSmallClosedInterval.Contains(1));
  EXPECT_TRUE(kSmallClosedInterval.Contains(3));
  EXPECT_FALSE(kSmallClosedInterval.Contains(4));

  EXPECT_TRUE((ClosedInterval<int>{1, 1}.Contains(1)));
  EXPECT_FALSE((ClosedInterval<int>{1, 1}.Contains(2)));
}

TEST(CoreMath, IsInsideRectangle) {
  const Vec2 kFakeRectangleTopLeft{5, 5};
  const Vec2 kFakeRectangleSize{2, 2};
  // Test the 4 conditions for the sides of the rectangle and the center.
  EXPECT_FALSE(IsInsideRectangle({4, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({6, 4}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({6, 8}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_FALSE(IsInsideRectangle({8, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));

  EXPECT_TRUE(IsInsideRectangle({5, 5}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({5, 7}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({7, 5}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({7, 7}, kFakeRectangleTopLeft, kFakeRectangleSize));
  EXPECT_TRUE(IsInsideRectangle({6, 6}, kFakeRectangleTopLeft, kFakeRectangleSize));
}

}  // namespace orbit_gl