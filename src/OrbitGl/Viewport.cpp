// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/Viewport.h"

#include <GteVector.h>

#include <cmath>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

Viewport::Viewport(int width, int height)
    : screen_width_(width), screen_height_(height), world_width_(width), world_height_(height) {}

void Viewport::Resize(int width, int height) {
  ORBIT_CHECK(width > 0);
  ORBIT_CHECK(height > 0);

  if (width == screen_width_ && height == screen_height_) return;

  screen_width_ = width;
  screen_height_ = height;
  FlagAsDirty();
}

int Viewport::GetScreenWidth() const { return screen_width_; }

int Viewport::GetScreenHeight() const { return screen_height_; }

void Viewport::SetWorldSize(float width, float height) {
  if (world_width_ == width && world_height_ == height) return;

  world_width_ = width;
  world_height_ = height;
  FlagAsDirty();
}

Vec2 Viewport::ScreenToWorld(const Vec2i& screen_coords) const {
  Vec2 world_coords;
  world_coords[0] = screen_coords[0] / static_cast<float>(screen_width_) * world_width_;
  world_coords[1] = screen_coords[1] / static_cast<float>(screen_height_) * world_height_;
  return world_coords;
}

Vec2i Viewport::WorldToScreen(const Vec2& world_coords) const {
  Vec2i screen_coords;
  screen_coords[0] =
      static_cast<int>(std::floor(world_coords[0] / world_width_ * GetScreenWidth()));
  screen_coords[1] =
      static_cast<int>(std::floor(world_coords[1] / world_height_ * GetScreenHeight()));
  return screen_coords;
}

}  // namespace orbit_gl