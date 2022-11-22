// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

TEST(Viewport, ResizingAndDirty) {
  Viewport viewport(100, 200);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  // Test initial values.
  EXPECT_EQ(viewport.GetScreenWidth(), 100);
  EXPECT_EQ(viewport.GetScreenHeight(), 200);
  EXPECT_EQ(viewport.GetWorldWidth(), 100.f);
  EXPECT_EQ(viewport.GetWorldHeight(), 200.f);

  // Test: Resizing should not affect the world size.
  viewport.Resize(1000, 2000);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();
  EXPECT_FALSE(viewport.IsDirty());

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetWorldWidth(), 100.f);
  EXPECT_EQ(viewport.GetWorldHeight(), 200.f);

  // Test: Changing the world size should not affect screen extents.
  viewport.SetWorldSize(500.f, 600.f);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetWorldWidth(), 500.f);
  EXPECT_EQ(viewport.GetWorldHeight(), 600.f);

  // Setting everything to the same values again should not mark the viewport as dirty.
  viewport.Resize(1000, 2000);
  viewport.SetWorldSize(500.f, 600.f);
  EXPECT_FALSE(viewport.IsDirty());
}

void VerifyConversion(Viewport& viewport, const Vec2i& screen_pos, const Vec2& world_pos) {
  EXPECT_EQ(viewport.ScreenToWorld(screen_pos), world_pos);
  EXPECT_EQ(viewport.WorldToScreen(world_pos), screen_pos);
}

TEST(Viewport, CoordinateConversion) {
  Viewport viewport(10, 100);
  viewport.SetWorldSize(10, 100);

  Vec2i screen_pos = Vec2i(8, 20);
  Vec2 world_pos = Vec2(8.f, 20.f);
  VerifyConversion(viewport, screen_pos, world_pos);

  // Change zoom: Zoom out to 200% horizontally, zoom in 50% vertically.
  viewport.SetWorldSize(20.f, 50.f);

  screen_pos = Vec2i(8, 20);
  world_pos = Vec2(16.f, 10.f);
  VerifyConversion(viewport, screen_pos, world_pos);
}

}  // namespace orbit_gl