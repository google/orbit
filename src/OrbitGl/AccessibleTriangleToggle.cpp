// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AccessibleTriangleToggle.h"

#include <string>

#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/TriangleToggle.h"

namespace orbit_gl {

AccessibleTriangleToggle::AccessibleTriangleToggle(TriangleToggle* triangle_toggle)
    : AccessibleCaptureViewElement(triangle_toggle, "TriangleToggle",
                                   orbit_accessibility::AccessibilityRole::Button),
      triangle_toggle_{triangle_toggle} {}

orbit_accessibility::AccessibilityState AccessibleTriangleToggle::AccessibleState() const {
  if (triangle_toggle_->IsCollapsible()) {
    return orbit_accessibility::AccessibilityState::Normal;
  }
  return orbit_accessibility::AccessibilityState::Disabled;
}

}  // namespace orbit_gl