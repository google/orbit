// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Viewport.h"

namespace orbit_gl {

TEST(Viewport, ResizingAndDirty) {
  Viewport viewport(100, 200);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  // Test initial values.
  EXPECT_EQ(viewport.GetScreenWidth(), 100);
  EXPECT_EQ(viewport.GetScreenHeight(), 200);
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(100, 200));
  EXPECT_EQ(viewport.GetVisibleWorldWidth(), 100.f);

  // Test: Resizing should not affect the world extents.
  viewport.Resize(1000, 2000);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();
  EXPECT_FALSE(viewport.IsDirty());

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetVisibleWorldWidth(), 100.f);
  EXPECT_EQ(viewport.GetVisibleWorldHeight(), 200.f);
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(100, 200));

  // Test: Changing the world width / height should not affect screen extents.
  viewport.SetVisibleWorldWidth(500.f);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  viewport.SetVisibleWorldHeight(600.f);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetVisibleWorldWidth(), 500.f);
  EXPECT_EQ(viewport.GetVisibleWorldHeight(), 600.f);
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(100, 200));

  // Test: Changing the world extent should not affect screen size and visible world size.
  viewport.SetWorldExtents(1000, 2000);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetVisibleWorldWidth(), 500.f);
  EXPECT_EQ(viewport.GetVisibleWorldHeight(), 600.f);
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(1000, 2000));

  // Setting everything to the same values again should not mark the viewport as dirty.
  viewport.Resize(1000, 2000);
  viewport.SetVisibleWorldWidth(500.f);
  viewport.SetVisibleWorldHeight(600.f);
  viewport.SetWorldExtents(1000, 2000);
  EXPECT_FALSE(viewport.IsDirty());
}

void VerifyConversion(Viewport& viewport, const Vec2i& screen_pos, const Vec2& world_pos,
                      const Vec2& world_size) {
  EXPECT_EQ(viewport.ScreenToWorldPos(screen_pos), world_pos);
  EXPECT_EQ(viewport.ScreenToWorldHeight(screen_pos[1]), world_size[1]);
  EXPECT_EQ(viewport.ScreenToWorldWidth(screen_pos[0]), world_size[0]);

  EXPECT_EQ(viewport.WorldToScreenPos(world_pos), screen_pos);
  EXPECT_EQ(viewport.WorldToScreenHeight(world_size[1]), screen_pos[1]);
  EXPECT_EQ(viewport.WorldToScreenWidth(world_size[0]), screen_pos[0]);
}

TEST(Viewport, CoordinateConversion) {
  Viewport viewport(10, 100);
  // Extents are large enough to scroll.
  viewport.SetWorldExtents(500, 500);

  Vec2i screen_pos = Vec2i(8, 20);
  Vec2 world_pos = Vec2(8.f, 20.f);
  Vec2 world_size = Vec2(8.f, 20.f);
  VerifyConversion(viewport, screen_pos, world_pos, world_size);

  // Change zoom: Zoom out to 200% horizontally, zoom in 50% vertically.
  viewport.SetVisibleWorldWidth(20.f);
  viewport.SetVisibleWorldHeight(50.f);

  screen_pos = Vec2i(8, 20);
  world_pos = Vec2(16.f, 10.f);
  world_size = Vec2(16.f, 10.f);
  VerifyConversion(viewport, screen_pos, world_pos, world_size);
}

}  // namespace orbit_gl