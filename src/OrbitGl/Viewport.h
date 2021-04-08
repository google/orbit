// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_VIEWPORT_H_
#define ORBIT_GL_VIEWPORT_H_

#include "CoreMath.h"

namespace orbit_gl {

// Defines a mapping from a 2D screen into a 2D (!) world. Provides functionality to convert between
// coordinate systems, taking scrolling and scaling into account.
//
// Uses the following coordinate systems:
//
// World:
//    +y
//    ^
//    |
//    |
//  (0, 0) ----> +x
//
// Screen:
//  (0, 0) ----> +x
//    |
//    |
//    v
//    +y
//
// where world(0, 0) is initially anchored at screen(0, 0), i.e. top left (meaning basically all
// world y coordinates are negative). Viewport clamps scrolling X values to be >= world_min, and
// scrollingY values to be <= world_min.
//
// Viewport will indicate if any changes happened that require redraw of the contents in between
// frames. See Viewport::IsDirty() for usage.
//
// NOTE: The screen coordinate system is different from what is referred to as "ScreenSpaceViewport"
// in GlCanvas! See TODO(b/177350599): Unify QtScreen and GlScreen

class Viewport {
 public:
  explicit Viewport(int width, int height);
  Viewport() = delete;

  // Size of the screen.
  void Resize(int width, int height);
  [[nodiscard]] int GetScreenWidth() const;
  [[nodiscard]] int GetScreenHeight() const;

  // Visible size of the world.
  void SetVisibleWorldWidth(float width);
  void SetVisibleWorldHeight(float height);
  [[nodiscard]] float GetVisibleWorldWidth() const;
  [[nodiscard]] float GetVisibleWorldHeight() const;

  // Absolute size of the world.
  void SetWorldExtents(float width, float height);
  [[nodiscard]] const Vec2& GetWorldExtents();

  // Minimum values of the world.
  void SetWorldMin(const Vec2& value);
  [[nodiscard]] const Vec2& GetWorldMin() const;

  // Position of world TopLeft anchored at (0, 0) ScreenSpace.
  void SetWorldTopLeftX(float x);
  void SetWorldTopLeftY(float y);
  [[nodiscard]] const Vec2& GetWorldTopLeft() const;

  [[nodiscard]] Vec2 ScreenToWorldPos(const Vec2i& screen_pos) const;

  // These methods to not take scrolling into account, use those if you need
  // to convert sizes instead of positions.
  [[nodiscard]] float ScreenToWorldWidth(int width) const;
  [[nodiscard]] float ScreenToWorldHeight(int height) const;

  [[nodiscard]] Vec2i WorldToScreenPos(const Vec2& world_pos) const;

  // These methods to not take scrolling into account, use those if you need
  // to convert sizes instead of positions.
  [[nodiscard]] int WorldToScreenWidth(float width) const;
  [[nodiscard]] int WorldToScreenHeight(float height) const;

  [[nodiscard]] Vec2i QtToGlScreenPos(const Vec2i& qt_pos) const;

  // "Dirty" indicates that any action has been performed that requires redraw of
  // the viewport contents. The flag must explicitely be cleared in each frame.
  [[nodiscard]] bool IsDirty() const { return is_dirty_; }
  void FlagAsDirty() { is_dirty_ = true; }
  void ClearDirtyFlag() { is_dirty_ = false; }

 private:
  int screen_width_;
  int screen_height_;

  float visible_world_width_;
  float visible_world_height_;

  Vec2 world_top_left_ = Vec2(0.f, 0.f);
  Vec2 world_extents_;
  Vec2 world_min_ = Vec2(0.f, 0.f);

  bool is_dirty_ = true;
};
}  // namespace orbit_gl

#endif