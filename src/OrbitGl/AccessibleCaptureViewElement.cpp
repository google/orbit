// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AccessibleCaptureViewElement.h"

#include <GteVector.h>

#include <algorithm>
#include <vector>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

int AccessibleCaptureViewElement::AccessibleChildCount() const {
  return capture_view_element_->GetNonHiddenChildren().size();
}

const orbit_accessibility::AccessibleInterface* AccessibleCaptureViewElement::AccessibleChild(
    int index) const {
  if (index < 0 || index >= AccessibleChildCount()) return nullptr;

  CaptureViewElement* child = capture_view_element_->GetNonHiddenChildren().at(index);
  return child->GetOrCreateAccessibleInterface();
}

const orbit_accessibility::AccessibleInterface* AccessibleCaptureViewElement::AccessibleParent()
    const {
  CaptureViewElement* parent = capture_view_element_->GetParent();
  if (parent == nullptr) {
    return nullptr;
  }
  return parent->GetOrCreateAccessibleInterface();
}
orbit_accessibility::AccessibilityRect AccessibleCaptureViewElement::AccessibleRect() const {
  ORBIT_CHECK(capture_view_element_ != nullptr);
  const Viewport* viewport = capture_view_element_->GetViewport();

  Vec2 pos = capture_view_element_->GetPos();
  Vec2 size = capture_view_element_->GetSize();

  Vec2i screen_pos = viewport->WorldToScreen(pos);
  Vec2i screen_size = viewport->WorldToScreen(size);

  // Adjust the coordinates to clamp the result to an on-screen rect
  // This will "cut" any part that is offscreen due to scrolling, and may result
  // in a final result with width / height of 0.

  // First: Clamp bottom
  if (screen_pos[1] + screen_size[1] > viewport->GetScreenHeight()) {
    screen_size[1] = std::max(0, viewport->GetScreenHeight() - static_cast<int>(screen_pos[1]));
  }
  // Second: Clamp top
  if (screen_pos[1] < 0) {
    screen_size[1] = std::max(0, static_cast<int>(screen_pos[1]) + screen_size[1]);
    screen_pos[1] = 0;
  }
  // Third: Clamp right
  if (screen_pos[0] + screen_size[0] > viewport->GetScreenWidth()) {
    screen_size[0] = std::max(0, viewport->GetScreenWidth() - static_cast<int>(screen_pos[0]));
  }
  // Fourth: Clamp left
  if (screen_pos[0] < 0) {
    screen_size[0] = std::max(0, static_cast<int>(screen_pos[0]) + screen_size[0]);
    screen_pos[0] = 0;
  }

  return {static_cast<int>(screen_pos[0]), static_cast<int>(screen_pos[1]), screen_size[0],
          screen_size[1]};
}

}  // namespace orbit_gl
