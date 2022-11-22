// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_
#define ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/AccessibleCaptureViewElement.h"

class TriangleToggle;

namespace orbit_gl {

/*
 * Accessibility implementation for (a track's) triangle toggle. The `TriangleToggle` is a visible
 * child of the track. It will be thus on the same level as the virtual elements for the tab and the
 * content (see `AccessibleTrack`).
 */
class AccessibleTriangleToggle : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTriangleToggle(TriangleToggle* triangle_toggle);

  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  TriangleToggle* triangle_toggle_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_TRIANGLE_TOGGLE_H_
