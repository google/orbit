// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Geometry.h"
#include "TranslationStack.h"

namespace orbit_gl {
static void ExpectEqualHasZ(const Vec2Z& expected, const Vec2Z& actual) {
  EXPECT_EQ(expected.shape, actual.shape);
  EXPECT_EQ(expected.z, actual.z);
}

TEST(TranslationStack, PushAndPop) {
  const Vec2Z orig{{0.5f, 0.5f}, 0.5f};
  const Vec2Z orig_result{{0, 0}, 0.5f};
  const Vec2Z trans{{1, 2}, 3};
  const Vec2Z trans_result{{1, 2}, 3.5f};

  TranslationStack stack;
  EXPECT_TRUE(stack.IsEmpty());

  Vec2Z result = stack.TranslateAndFloorVertex(orig);
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