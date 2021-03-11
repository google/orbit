// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_VIEWPORT_H_
#define ORBIT_GL_VIEWPORT_H_

#include "CoreMath.h"

namespace orbit_gl {

/**
Defines a mapping from a 2D screen into a 2D (!) world. Provides functionality to convert between
coordinate systems, taking scrolling and scaling into account.

Uses the following coordinate systems:

World:
    ^
    |
    |
  (0, 0) ---->

Screen:
  (0, 0) ---->
    |
    |
    v

where world(0, 0) is initially anchored at screen(0, 0), i.e. top left (meaning basically all world
y coordinates are negative). Viewport clamps all X values to be >= 0, and all Y values to be <= 0.

See TODO(b/177350599): Unify QtScreen and GlScreen
**/
class Viewport {
 public:
  explicit Viewport(int width, int height);

  void Resize(int width, int height);
  [[nodiscard]] int GetWidth() const;
  [[nodiscard]] int GetHeight() const;

  // Visible size of the world
  void SetWorldWidth(float width);
  void SetWorldHeight(float height);
  [[nodiscard]] float GetWorldWidth() const;
  [[nodiscard]] float GetWorldHeight() const;

  // Absolute size of the world
  void SetWorldExtents(float width, float height);
  [[nodiscard]] Vec2 GetWorldExtents();

  void SetWorldTopLeftY(float y);
  void SetWorldTopLeftX(float x);
  [[nodiscard]] const Vec2& GetWorldTopLeft() const;

  [[nodiscard]] Vec2 ScreenToWorldPos(const Vec2i& screen_pos) const;
  [[nodiscard]] float ScreenToWorldHeight(int height) const;
  [[nodiscard]] float ScreenToWorldWidth(int width) const;

  [[nodiscard]] Vec2i WorldToScreenPos(const Vec2& world_pos) const;
  [[nodiscard]] int WorldToScreenHeight(float height) const;
  [[nodiscard]] int WorldToScreenWidth(float width) const;

  [[nodiscard]] Vec2i QtToGlScreenPos(const Vec2i& qt_pos) const;

  [[nodiscard]] bool IsDirty() const { return is_dirty_; }
  void FlagAsDirty() { is_dirty_ = true; }
  void ClearDirtyFlag() { is_dirty_ = false; }

 private:
  int width_;
  int height_;

  float world_width_;
  float world_height_;

  Vec2 world_top_left_ = Vec2(0.f, 0.f);
  Vec2 world_extents_;

  bool is_dirty_ = true;
};
}  // namespace orbit_gl

#endif