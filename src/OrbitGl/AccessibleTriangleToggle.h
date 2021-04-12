// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_
#define ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_

#include "AccessibleCaptureViewElement.h"
#include "CaptureViewElement.h"
#include "Track.h"

class TriangleToggle;

namespace orbit_gl {

/*
 * Accessibility implementation for (a track's) triangle toggle. The `TriangleToggle` is a visible
 * child of the track. It will be thus on the same level as the virtual elements for the tab and the
 * content (see `AccessibleTrack`).
 */
class AccessibleTriangleToggle : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTriangleToggle(TriangleToggle* triangle_toggle, Track* parent);

  [[nodiscard]] int AccessibleChildCount() const override { return 0; };
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override {
    return nullptr;
  }

  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override { return "TriangleToggle"; }

  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Button;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  TriangleToggle* triangle_toggle_;
  Track* parent_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_
