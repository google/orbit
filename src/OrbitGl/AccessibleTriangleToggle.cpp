// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTriangleToggle.h"

#include "CoreMath.h"
#include "GlCanvas.h"

namespace orbit_gl {

AccessibleTriangleToggle::AccessibleTriangleToggle(TriangleToggle* triangle_toggle, Track* parent)
    : AccessibleCaptureViewElement(triangle_toggle),
      triangle_toggle_{triangle_toggle},
      parent_{parent} {}

orbit_accessibility::AccessibilityState AccessibleTriangleToggle::AccessibleState() const {
  if (triangle_toggle_->IsCollapsible()) {
    return orbit_accessibility::AccessibilityState::Normal;
  }
  return orbit_accessibility::AccessibilityState::Disabled;
}

const orbit_accessibility::AccessibleInterface* AccessibleTriangleToggle::AccessibleParent() const {
  // Hack: The toggle's parent is the track tab, not the track itself.
  // As the tab is virtual only, we expose it's accessibility interface here
  // directly.
  return parent_->GetOrCreateAccessibleInterface()->AccessibleChild(0);
}

}  // namespace orbit_gl