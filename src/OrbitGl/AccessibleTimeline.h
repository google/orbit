// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TIMELINE_H_
#define ORBIT_GL_ACCESSIBLE_TIMELINE_H_

#include "AccessibleCaptureViewElement.h"

namespace orbit_gl {

/*
 * Accessibility information for the timeline.
 */
class AccessibleTimeline : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTimeline(CaptureViewElement* timeline)
      : AccessibleCaptureViewElement(timeline){};

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int /*index*/) const override {
    return nullptr;
  }

  [[nodiscard]] std::string AccessibleName() const override { return "Timeline"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Pane;
  }
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_TIMELINE_H_
