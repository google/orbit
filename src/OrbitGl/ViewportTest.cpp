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
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(0.f, 0.f));

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
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(0.f, 0.f));

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
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(0.f, 0.f));
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(100, 200));

  // Test: Changing the world extent should not affect screen size and visible world size.
  viewport.SetWorldExtents(1000, 2000);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  EXPECT_EQ(viewport.GetScreenWidth(), 1000);
  EXPECT_EQ(viewport.GetScreenHeight(), 2000);
  EXPECT_EQ(viewport.GetVisibleWorldWidth(), 500.f);
  EXPECT_EQ(viewport.GetVisibleWorldHeight(), 600.f);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(0.f, 0.f));
  EXPECT_EQ(viewport.GetWorldExtents(), Vec2(1000, 2000));

  // Test: Scrolling (within correct range) should mark as dirty.
  viewport.Resize(100, 200);
  viewport.ClearDirtyFlag();

  viewport.SetWorldTopLeftX(100.f);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  viewport.SetWorldTopLeftY(-50.f);
  EXPECT_TRUE(viewport.IsDirty());
  viewport.ClearDirtyFlag();

  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100.f, -50.f));

  // Setting everything to the same values again should not mark the viewport as dirty.
  viewport.Resize(100, 200);
  viewport.SetVisibleWorldWidth(500.f);
  viewport.SetVisibleWorldHeight(600.f);
  viewport.SetWorldTopLeftX(100.f);
  viewport.SetWorldTopLeftY(-50.f);
  viewport.SetWorldExtents(1000, 2000);
  EXPECT_FALSE(viewport.IsDirty());
}

TEST(Viewport, ScrollingWithoutZoom) {
  Viewport viewport(100, 200);
  viewport.SetWorldExtents(200, 300);

  viewport.SetWorldTopLeftX(100.f);
  viewport.SetWorldTopLeftY(-50.f);

  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100.f, -50.f));

  viewport.SetWorldTopLeftX(150.f);
  viewport.SetWorldTopLeftY(-150.f);

  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100.f, -100.f));
  EXPECT_EQ(viewport.IsDirty(), true);
  viewport.ClearDirtyFlag();

  // Scrolling further after clamping should not result in a dirty flag.
  viewport.SetWorldTopLeftX(200.f);
  viewport.SetWorldTopLeftY(-200.f);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100.f, -100.f));
  EXPECT_EQ(viewport.IsDirty(), false);

  // Resizing should clamp.
  viewport.SetWorldExtents(150, 250);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(50.f, -50.f));

  // Changing world min should adjust current top left...
  viewport.SetWorldMin(Vec2(100, -100));
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100, -100));
  viewport.SetWorldTopLeftX(0);
  viewport.SetWorldTopLeftY(0);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(100, -100));

  // ... and is taken into account when scrolling.
  viewport.SetWorldTopLeftX(500);
  viewport.SetWorldTopLeftY(-500);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(150, -150));
}

TEST(Viewport, ScrollingWithZoom) {
  Viewport viewport(100, 200);
  viewport.SetVisibleWorldWidth(200);
  viewport.SetVisibleWorldHeight(400);
  viewport.SetWorldExtents(400, 800);

  viewport.SetWorldTopLeftX(250.f);
  viewport.SetWorldTopLeftY(-350.f);

  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(200.f, -350.f));

  viewport.SetWorldTopLeftX(300.f);
  viewport.SetWorldTopLeftY(-450.f);

  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(200.f, -400.f));
  EXPECT_EQ(viewport.IsDirty(), true);
  viewport.ClearDirtyFlag();

  // Scrolling further after clamping should not result in a dirty flag.
  viewport.SetWorldTopLeftX(350.f);
  viewport.SetWorldTopLeftY(-600.f);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(200.f, -400.f));
  EXPECT_EQ(viewport.IsDirty(), false);

  // Resizing should clamp.
  viewport.SetWorldExtents(250, 450);
  EXPECT_EQ(viewport.GetWorldTopLeft(), Vec2(50.f, -50.f));
}

void VerifyConversion(Viewport& viewport, const Vec2i& screen_pos, const Vec2& world_pos,
                      const Vec2& world_size) {
  EXPECT_EQ(viewport.ScreenToWorldPos(screen_pos), world_pos);
  EXPECT_EQ(viewport.ScreenToWorldHeight(screen_pos[1]), world_size[1]);
  EXPECT_EQ(viewport.ScreenToWorldWidth(screen_pos[0]), world_size[0]);

  EXPECT_EQ(viewport.WorldToScreenPos(world_pos), screen_pos);
  EXPECT_EQ(viewport.WorldToScreenHeight(world_size[1]), screen_pos[1]);
  EXPECT_EQ(viewport.WorldToScreenWidth(world_size[0]), screen_pos[0]);

  EXPECT_EQ(viewport.QtToGlScreenPos(screen_pos),
            Vec2i(screen_pos[0], viewport.GetScreenHeight() - screen_pos[1]));
}

TEST(Viewport, CoordinateConversion) {
  Viewport viewport(10, 100);
  // Extents are large enough to scroll.
  viewport.SetWorldExtents(500, 500);

  Vec2i screen_pos = Vec2i(8, 20);
  Vec2 world_pos = Vec2(8.f, -20.f);
  Vec2 world_size = Vec2(8.f, 20.f);
  VerifyConversion(viewport, screen_pos, world_pos, world_size);

  // Change zoom: Zoom out to 200% horizontally, zoom in 50% vertically.
  viewport.SetVisibleWorldWidth(20.f);
  viewport.SetVisibleWorldHeight(50.f);

  screen_pos = Vec2i(8, 20);
  world_pos = Vec2(16.f, -10.f);
  world_size = Vec2(16.f, 10.f);
  VerifyConversion(viewport, screen_pos, world_pos, world_size);

  viewport.SetWorldTopLeftX(10.f);
  viewport.SetWorldTopLeftY(-100.f);
  world_pos = Vec2(26.f, -110.f);
  VerifyConversion(viewport, screen_pos, world_pos, world_size);
}

}  // namespace orbit_gl