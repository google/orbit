// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/ThreadBar.h"

#include "OrbitGl/AccessibleCaptureViewElement.h"

namespace orbit_gl {

std::unique_ptr<orbit_accessibility::AccessibleInterface> ThreadBar::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureViewElement>(
      this, GetName(), orbit_accessibility::AccessibilityRole::Pane,
      orbit_accessibility::AccessibilityState::Focusable);
}

}  // namespace orbit_gl
