// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTriangleToggle.h"

#include "CoreMath.h"
#include "GlCanvas.h"

namespace orbit_gl {

orbit_accessibility::AccessibilityRect AccessibleTriangleToggle::AccessibleLocalRect() const {
  CHECK(triangle_toggle_ != nullptr);
  CHECK(triangle_toggle_->GetCanvas() != nullptr);

  GlCanvas* canvas = triangle_toggle_->GetCanvas();
  Vec2 pos = triangle_toggle_->GetPos();
  Vec2 size = triangle_toggle_->GetSize();

  Vec2 screen_pos = canvas->WorldToScreen(pos);
  int screen_width = canvas->WorldToScreenWidth(size[0]);
  int screen_height = canvas->WorldToScreenHeight(size[1]);

  // Adjust the coordinates to clamp the result to an on-screen rect
  // This will "cut" any part that is offscreen due to scrolling, and may result
  // in a final result with width / height of 0.

  // First: Clamp bottom
  if (screen_pos[1] + screen_height > canvas->GetHeight()) {
    screen_height = std::max(0, canvas->GetHeight() - static_cast<int>(screen_pos[1]));
  }
  // Second: Clamp top
  if (screen_pos[1] < 0) {
    screen_height = std::max(0, static_cast<int>(screen_pos[1]) + screen_height);
    screen_pos[1] = 0;
  }

  orbit_accessibility::AccessibilityRect parent_rect =
      parent_->GetOrCreateAccessibleInterface()->AccessibleLocalRect();

  return orbit_accessibility::AccessibilityRect(static_cast<int>(screen_pos[0]) - parent_rect.left,
                                                static_cast<int>(screen_pos[1]) - parent_rect.top,
                                                screen_width, screen_height);
}

orbit_accessibility::AccessibilityState AccessibleTriangleToggle::AccessibleState() const {
  if (triangle_toggle_->IsInactive()) {
    return orbit_accessibility::AccessibilityState::Disabled;
  }
  return orbit_accessibility::AccessibilityState::Normal;
}

const orbit_accessibility::AccessibleInterface* AccessibleTriangleToggle::AccessibleParent() const {
  return parent_->GetOrCreateAccessibleInterface();
}

}  // namespace orbit_gl