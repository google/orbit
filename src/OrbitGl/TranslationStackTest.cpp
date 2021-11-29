// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Geometry.h"
#include "TranslationStack.h"

namespace orbit_gl {
TEST(TranslationStack, PushAndPop) {
  const Vec3 orig(0.5f, 0.5f, 0.5f);
  const Vec3 orig_result(0, 0, 0.5f);
  const Vec3 trans(1, 2, 3);
  const Vec3 trans_result(1, 2, 3.5f);

  TranslationStack stack;
  EXPECT_TRUE(stack.IsEmpty());

  Vec3 result = stack.TranslateAndFloorVertex(orig);
  EXPECT_EQ(orig_result, result);

  stack.PushTranslation(trans[0], trans[1], trans[2]);
  result = stack.TranslateAndFloorVertex(orig);
  EXPECT_EQ(trans_result, result);

  stack.PopTranslation();
  result = stack.TranslateAndFloorVertex(orig);
  EXPECT_EQ(orig_result, result);
}

TEST(TranslationStack, RaisesOnError) {
  TranslationStack stack;
  EXPECT_DEATH(stack.PopTranslation(), "Check failed");

  stack.PushTranslation(0, 0, 0);
  stack.PopTranslation();
  EXPECT_DEATH(stack.PopTranslation(), "Check failed");
}

}  // namespace orbit_gl