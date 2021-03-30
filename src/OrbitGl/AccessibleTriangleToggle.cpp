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
  // Hack: The toggle's parent is the track tab, not the track itself.
  // As the tab is virtual only, we expose it's accessibility interface here
  // directly.
  return parent_->GetOrCreateAccessibleInterface()->AccessibleChild(0);
}

}  // namespace orbit_gl