// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Geometry.h"
#include "TranslationStack.h"

namespace orbit_gl {
static void ExpectEqualHasZ(const HasZ<Vec2>& expected, const HasZ<Vec2>& actual) {
  EXPECT_EQ(expected.shape, actual.shape);
  EXPECT_EQ(expected.z, actual.z);
}

TEST(TranslationStack, PushAndPop) {
  const HasZ<Vec2> orig{{0.5f, 0.5f}, 0.5f};
  const HasZ<Vec2> orig_result{{0, 0}, 0.5f};
  const HasZ<Vec2> trans{{1, 2}, 3};
  const HasZ<Vec2> trans_result{{1, 2}, 3.5f};

  TranslationStack stack;
  EXPECT_TRUE(stack.IsEmpty());

  HasZ<Vec2> result = stack.TranslateAndFloorVertex(orig);
  ExpectEqualHasZ(orig_result, result);

  stack.PushTranslation(trans.shape[0], trans.shape[1], trans.z);
  result = stack.TranslateAndFloorVertex(orig);
  ExpectEqualHasZ(trans_result, result);

  stack.PopTranslation();
  result = stack.TranslateAndFloorVertex(orig);
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