// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

template <typename T>
struct ClosedIntervalTest : public testing::Test {
  using MyParamType = T;
};

using MyTypes = testing::Types<int, float>;
TYPED_TEST_SUITE(ClosedIntervalTest, MyTypes);

TYPED_TEST(ClosedIntervalTest, Intersects) {
  using Type = typename TestFixture::MyParamType;
  EXPECT_TRUE((ClosedInterval<Type>{0, 3}.Intersects(ClosedInterval<Type>{1, 2})));
  EXPECT_TRUE((ClosedInterval<Type>{1, 2}.Intersects(ClosedInterval<Type>{0, 3})));
  EXPECT_TRUE((ClosedInterval<Type>{0, 1}.Intersects(ClosedInterval<Type>{1, 2})));
  EXPECT_TRUE((ClosedInterval<Type>{1, 2}.Intersects(ClosedInterval<Type>{0, 1})));
  EXPECT_FALSE((ClosedInterval<Type>{0, 1}.Intersects(ClosedInterval<Type>{2, 3})));
  EXPECT_FALSE((ClosedInterval<Type>{2, 3}.Intersects(ClosedInterval<Type>{0, 1})));

  const ClosedInterval<Type> small_closed_interval{0, 3};
  EXPECT_TRUE(small_closed_interval.Intersects(ClosedInterval<Type>{1, 1}));
  EXPECT_TRUE(small_closed_interval.Intersects(ClosedInterval<Type>{3, 3}));
  EXPECT_FALSE(small_closed_interval.Intersects(ClosedInterval<Type>{4, 4}));
}

TYPED_TEST(ClosedIntervalTest, Contains) {
  using Type = typename TestFixture::MyParamType;
  const ClosedInterval<Type> small_closed_interval{0, 3};
  EXPECT_FALSE(small_closed_interval.Contains(-1));
  EXPECT_TRUE(small_closed_interval.Contains(0));
  EXPECT_TRUE(small_closed_interval.Contains(1));
  EXPECT_TRUE(small_closed_interval.Contains(3));
  EXPECT_FALSE(small_closed_interval.Contains(4));

  EXPECT_TRUE((ClosedInterval<Type>{1, 1}.Contains(1)));
  EXPECT_FALSE((ClosedInterval<Type>{1, 1}.Contains(2)));
}

TEST(CoreMath, IsInsideRectangle) {
  const Vec2 fake_rectangle_top_left{5, 5};
  const Vec2 fake_rectangle_size{2, 2};
  // Test the 4 conditions for the sides of the rectangle and the center.
  EXPECT_FALSE(IsInsideRectangle({4, 6}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_FALSE(IsInsideRectangle({6, 4}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_FALSE(IsInsideRectangle({6, 8}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_FALSE(IsInsideRectangle({8, 6}, fake_rectangle_top_left, fake_rectangle_size));

  EXPECT_TRUE(IsInsideRectangle({5, 5}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_TRUE(IsInsideRectangle({5, 7}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_TRUE(IsInsideRectangle({7, 5}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_TRUE(IsInsideRectangle({7, 7}, fake_rectangle_top_left, fake_rectangle_size));
  EXPECT_TRUE(IsInsideRectangle({6, 6}, fake_rectangle_top_left, fake_rectangle_size));
}

}  // namespace orbit_gl