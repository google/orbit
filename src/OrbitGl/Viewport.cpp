// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Viewport.h"

#include <algorithm>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

Viewport::Viewport(int width, int height)
    : screen_width_(width),
      screen_height_(height),
      visible_world_width_(width),
      visible_world_height_(height),
      world_extents_(Vec2(width, height)) {}

void Viewport::Resize(int width, int height) {
  CHECK(width > 0);
  CHECK(height > 0);

  if (width == screen_width_ && height == screen_height_) return;

  screen_width_ = width;
  screen_height_ = height;
  FlagAsDirty();
}

int Viewport::GetScreenWidth() const { return screen_width_; }

int Viewport::GetScreenHeight() const { return screen_height_; }

void Viewport::SetVisibleWorldWidth(float width) {
  if (width == visible_world_width_) return;

  visible_world_width_ = width;
  // Recalculate required scrolling.
  SetWorldTopLeftX(world_top_left_[0]);
  FlagAsDirty();
}

float Viewport::GetVisibleWorldWidth() const { return visible_world_width_; }

void Viewport::SetVisibleWorldHeight(float height) {
  if (height == visible_world_height_) return;

  visible_world_height_ = height;
  // Recalculate required scrolling.
  SetWorldTopLeftY(world_top_left_[1]);

  FlagAsDirty();
}

float Viewport::GetVisibleWorldHeight() const { return visible_world_height_; }

void Viewport::SetWorldExtents(float width, float height) {
  Vec2 size = Vec2(width, height);

  if (size == world_extents_) return;

  world_extents_ = size;
  // Recalculate required scrolling.
  SetWorldTopLeftX(world_top_left_[0]);
  SetWorldTopLeftY(world_top_left_[1]);

  FlagAsDirty();
}

const Vec2& Viewport::GetWorldExtents() { return world_extents_; }

void Viewport::SetWorldMin(const Vec2& value) {
  world_min_ = value;
  SetWorldTopLeftX(world_top_left_[0]);
  SetWorldTopLeftY(world_top_left_[1]);
}

const Vec2& Viewport::GetWorldMin() const { return world_min_; }

void Viewport::SetWorldTopLeftY(float y) {
  float clamped = std::min(std::max(y, visible_world_height_ - world_extents_[1] + world_min_[1]),
                           world_min_[1]);
  if (world_top_left_[1] == clamped) return;

  world_top_left_[1] = clamped;
  FlagAsDirty();
}

void Viewport::SetWorldTopLeftX(float x) {
  float clamped = std::max(std::min(x, world_extents_[0] - visible_world_width_ + world_min_[0]),
                           world_min_[0]);
  if (world_top_left_[0] == clamped) return;

  world_top_left_[0] = clamped;
  FlagAsDirty();
}

const Vec2& Viewport::GetWorldTopLeft() const { return world_top_left_; }

Vec2 Viewport::ScreenToWorldPos(const Vec2i& screen_pos) const {
  Vec2 world_pos;
  world_pos[0] =
      world_top_left_[0] + screen_pos[0] / static_cast<float>(screen_width_) * visible_world_width_;
  world_pos[1] = world_top_left_[1] -
                 screen_pos[1] / static_cast<float>(screen_height_) * visible_world_height_;
  return world_pos;
}

float Viewport::ScreenToWorldHeight(int height) const {
  return (static_cast<float>(height) / static_cast<float>(screen_height_)) * visible_world_height_;
}

float Viewport::ScreenToWorldWidth(int width) const {
  return (static_cast<float>(width) / static_cast<float>(screen_width_)) * visible_world_width_;
}

Vec2i Viewport::WorldToScreenPos(const Vec2& world_pos) const {
  Vec2i screen_pos;
  screen_pos[0] = static_cast<int>(
      floorf((world_pos[0] - world_top_left_[0]) / visible_world_width_ * GetScreenWidth()));
  screen_pos[1] = static_cast<int>(
      floorf((world_top_left_[1] - world_pos[1]) / visible_world_height_ * GetScreenHeight()));
  return screen_pos;
}

int Viewport::WorldToScreenHeight(float height) const {
  return static_cast<int>(height / visible_world_height_ * GetScreenHeight());
}

int Viewport::WorldToScreenWidth(float width) const {
  return static_cast<int>(width / visible_world_width_ * GetScreenWidth());
}

// TODO(b/177350599): Unify QtScreen and GlScreen
// QtScreen(x,y) --> GlScreen(x,height-y)
Vec2i Viewport::QtToGlScreenPos(const Vec2i& qt_pos) const {
  Vec2i gl_pos = qt_pos;
  gl_pos[1] = screen_height_ - qt_pos[1];
  return gl_pos;
}

}  // namespace orbit_gl