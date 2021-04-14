// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleCaptureViewElement.h"

#include "GlCanvas.h"
#include "Viewport.h"

namespace orbit_gl {
const orbit_accessibility::AccessibleInterface* AccessibleCaptureViewElement::AccessibleParent()
    const {
  CaptureViewElement* parent = capture_view_element_->GetParent();
  if (parent == nullptr) {
    return nullptr;
  }
  return parent->GetOrCreateAccessibleInterface();
}
orbit_accessibility::AccessibilityRect AccessibleCaptureViewElement::AccessibleRect() const {
  CHECK(capture_view_element_ != nullptr);
  GlCanvas* canvas = capture_view_element_->GetCanvas();
  CHECK(canvas != nullptr);
  const Viewport& viewport = canvas->GetViewport();

  Vec2 pos = capture_view_element_->GetPos();
  Vec2 size = capture_view_element_->GetSize();

  Vec2i screen_pos = viewport.WorldToScreenPos(pos);
  int screen_width = viewport.WorldToScreenWidth(size[0]);
  int screen_height = viewport.WorldToScreenHeight(size[1]);

  // Adjust the coordinates to clamp the result to an on-screen rect
  // This will "cut" any part that is offscreen due to scrolling, and may result
  // in a final result with width / height of 0.

  // First: Clamp bottom
  if (screen_pos[1] + screen_height > viewport.GetScreenHeight()) {
    screen_height = std::max(0, viewport.GetScreenHeight() - static_cast<int>(screen_pos[1]));
  }
  // Second: Clamp top
  if (screen_pos[1] < 0) {
    screen_height = std::max(0, static_cast<int>(screen_pos[1]) + screen_height);
    screen_pos[1] = 0;
  }
  // Third: Clamp right
  if (screen_pos[0] + screen_width > viewport.GetScreenWidth()) {
    screen_width = std::max(0, viewport.GetScreenWidth() - static_cast<int>(screen_pos[0]));
  }
  // Fourth: Clamp left
  if (screen_pos[0] < 0) {
    screen_width = std::max(0, static_cast<int>(screen_pos[0]) + screen_width);
    screen_pos[0] = 0;
  }

  return orbit_accessibility::AccessibilityRect(static_cast<int>(screen_pos[0]),
                                                static_cast<int>(screen_pos[1]), screen_width,
                                                screen_height);
}

}  // namespace orbit_gl
