// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_VIEWPORT_H_
#define ORBIT_GL_VIEWPORT_H_

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

// Defines a mapping from pixel space to world space. World space may use a different scaling
// in x- or y-direction. Main use case of this class is to convert coordinates when interacting
// with mouse input.
//
// Unlike in earlier versions, this class does not take care of positioning the viewport within
// the world. Our "world" is always anchored at (0, 0) top left using the following coordinates:
//
// (0, 0) ------> +x
//    |
//    |
//    v
//    +y
//
// When the size of the world and the screen differs, the contents of the world are scaled up
// or down accordingly to fill the whole screen.
//
// Viewport will indicate if any changes happened that require redraw of the contents in between
// frames. See Viewport::IsDirty() for usage.

class Viewport {
 public:
  explicit Viewport(int width, int height);
  Viewport() = delete;

  // Size of the screen.
  void Resize(int width, int height);
  [[nodiscard]] int GetScreenWidth() const;
  [[nodiscard]] int GetScreenHeight() const;

  // Size of the world.
  void SetWorldSize(float width, float height);
  [[nodiscard]] const float GetWorldWidth() const { return world_width_; }
  [[nodiscard]] const float GetWorldHeight() const { return world_height_; }

  // Convert between screen and world space. This method works for positions and sizes since there
  // is no translation component involved, only scaling.
  [[nodiscard]] Vec2 ScreenToWorld(const Vec2i& screen_coords) const;
  // Convert between world and screen space. This method works for positions and sizes since there
  // is no translation component involved, only scaling.
  [[nodiscard]] Vec2i WorldToScreen(const Vec2& world_coords) const;

  // "Dirty" indicates that any action has been performed that requires redraw of
  // the viewport contents. The flag must explicitely be cleared in each frame.
  [[nodiscard]] bool IsDirty() const { return is_dirty_; }
  void FlagAsDirty() { is_dirty_ = true; }
  void ClearDirtyFlag() { is_dirty_ = false; }

 private:
  int screen_width_;
  int screen_height_;

  float world_width_;
  float world_height_;

  bool is_dirty_ = true;
};
}  // namespace orbit_gl

#endif