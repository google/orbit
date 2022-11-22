// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>

#include <memory>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TranslationStack.h"

namespace orbit_gl {
static void ExpectEqualHasZ(const LayeredVec2& expected, const LayeredVec2& actual) {
  EXPECT_EQ(expected.xy, actual.xy);
  EXPECT_EQ(expected.z, actual.z);
}

TEST(TranslationStack, PushAndPop) {
  const LayeredVec2 orig{{0.5f, 0.5f}, 0.5f};
  const LayeredVec2 orig_result{{0, 0}, 0.5f};
  const LayeredVec2 trans{{1, 2}, 3};
  const LayeredVec2 trans_result{{1, 2}, 3.5f};

  TranslationStack stack;
  EXPECT_TRUE(stack.IsEmpty());

  LayeredVec2 result = stack.TranslateXYZAndFloorXY(orig);
  ExpectEqualHasZ(orig_result, result);

  stack.PushTranslation(trans.xy[0], trans.xy[1], trans.z);
  result = stack.TranslateXYZAndFloorXY(orig);
  ExpectEqualHasZ(trans_result, result);

  stack.PopTranslation();
  result = stack.TranslateXYZAndFloorXY(orig);
  ExpectEqualHasZ(orig_result, result);
}

TEST(TranslationStack, RaisesOnError) {
  TranslationStack stack;
  EXPECT_DEATH(stack.PopTranslation(), "Check failed");

  stack.PushTranslation(0, 0, 0);
  stack.PopTranslation();
  EXPECT_DEATH(stack.PopTranslation(), "Check failed");
}

}  // namespace orbit_gl