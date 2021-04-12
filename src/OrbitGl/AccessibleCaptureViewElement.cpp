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
orbit_accessibility::AccessibilityRect AccessibleCaptureViewElement::AccessibleLocalRect() const {
  Vec2 pos = capture_view_element_->GetPos();
  Vec2 local_pos;

  const CaptureViewElement* parent = capture_view_element_->GetParent();
  // TODO(b/177350599): This could be cleaned up with clearer coordinate systems
  if (parent != nullptr) {
    Vec2 parent_pos = parent->GetPos();
    local_pos = Vec2(pos[0] - parent_pos[0], parent_pos[1] - pos[1]);
  } else {
    local_pos = pos;
  }

  GlCanvas* canvas = capture_view_element_->GetCanvas();
  CHECK(canvas != nullptr);
  const Viewport& viewport = canvas->GetViewport();

  return orbit_accessibility::AccessibilityRect(
      viewport.WorldToScreenWidth(local_pos[0]), viewport.WorldToScreenHeight(local_pos[1]),
      viewport.WorldToScreenWidth(capture_view_element_->GetSize()[0]),
      viewport.WorldToScreenHeight(capture_view_element_->GetSize()[1]));
}

}  // namespace orbit_gl
