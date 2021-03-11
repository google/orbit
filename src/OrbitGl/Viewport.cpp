// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Viewport.h"

#include <algorithm>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

Viewport::Viewport(int width, int height) : width_(width), height_(height) {
  world_width_ = width;
  world_height_ = height;
  world_extents_ = Vec2(width, height);
}

void Viewport::Resize(int width, int height) {
  CHECK(width > 0);
  CHECK(height > 0);

  if (width == width_ && height == height_) return;

  width_ = width;
  height_ = height;
  FlagAsDirty();
}

int Viewport::GetWidth() const { return width_; }

int Viewport::GetHeight() const { return height_; }

void Viewport::SetWorldWidth(float width) {
  if (width == world_width_) return;

  world_width_ = width;
  // Recalculate required scrolling
  SetWorldTopLeftX(world_top_left_[0]);
  FlagAsDirty();
}

float Viewport::GetWorldWidth() const { return world_width_; }

void Viewport::SetWorldHeight(float height) {
  if (height == world_height_) return;

  world_height_ = height;
  // Recalculate required scrolling
  SetWorldTopLeftY(world_top_left_[1]);

  FlagAsDirty();
}

float Viewport::GetWorldHeight() const { return world_height_; }

void Viewport::SetWorldExtents(float width, float height) {
  Vec2 size = Vec2(width, height);

  if (size == world_extents_) return;

  world_extents_ = size;
  // Recalculate required scrolling
  SetWorldTopLeftX(world_top_left_[0]);
  SetWorldTopLeftY(world_top_left_[1]);

  FlagAsDirty();
}

Vec2 Viewport::GetWorldExtents() { return world_extents_; }

void Viewport::SetWorldTopLeftY(float y) {
  float clamped = std::min(std::max(y, world_height_ - world_extents_[1]), 0.f);
  if (world_top_left_[1] == clamped) return;

  world_top_left_[1] = clamped;
  FlagAsDirty();
}

void Viewport::SetWorldTopLeftX(float x) {
  float clamped = std::max(std::min(x, world_extents_[0] - world_width_), 0.f);
  if (world_top_left_[0] == clamped) return;

  world_top_left_[0] = clamped;
  FlagAsDirty();
}

const Vec2& Viewport::GetWorldTopLeft() const { return world_top_left_; }

Vec2 Viewport::ScreenToWorldPos(const Vec2i& screen_pos) const {
  Vec2 world_pos;
  world_pos[0] = world_top_left_[0] + screen_pos[0] / static_cast<float>(width_) * world_width_;
  world_pos[1] = world_top_left_[1] - screen_pos[1] / static_cast<float>(height_) * world_height_;
  return world_pos;
}

float Viewport::ScreenToWorldHeight(int height) const {
  return (static_cast<float>(height) / static_cast<float>(height_)) * world_height_;
}

float Viewport::ScreenToWorldWidth(int width) const {
  return (static_cast<float>(width) / static_cast<float>(width_)) * world_width_;
}

Vec2i Viewport::WorldToScreenPos(const Vec2& world_pos) const {
  Vec2i screen_pos;
  screen_pos[0] =
      static_cast<int>(floorf((world_pos[0] - world_top_left_[0]) / world_width_ * GetWidth()));
  screen_pos[1] =
      static_cast<int>(floorf((world_top_left_[1] - world_pos[1]) / world_height_ * GetHeight()));
  return screen_pos;
}

int Viewport::WorldToScreenHeight(float height) const {
  return static_cast<int>(height / world_height_ * GetHeight());
}

int Viewport::WorldToScreenWidth(float width) const {
  return static_cast<int>(width / world_width_ * GetWidth());
}

// TODO (b/177350599): Unify QtScreen and GlScreen
// QtScreen(x,y) --> GlScreen(x,height-y)
Vec2i Viewport::QtToGlScreenPos(const Vec2i& qt_pos) const {
  Vec2i gl_pos = qt_pos;
  gl_pos[1] = height_ - qt_pos[1];
  return gl_pos;
}

}  // namespace orbit_gl