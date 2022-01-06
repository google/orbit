// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TIMELINE_H_
#define ORBIT_GL_ACCESSIBLE_TIMELINE_H_

#include "AccessibleCaptureViewElement.h"

class TimelineUi;

namespace orbit_gl {

/*
 * Accessibility information for the timeline.
 */
class AccessibleTimeline : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTimeline(TimelineUi* timeline_ui)
      : AccessibleCaptureViewElement(timeline_ui){};

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int /*index*/) const override {
    return nullptr;
  }

  [[nodiscard]] std::string AccessibleName() const override { return "Timeline"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Graphic;  // TODO: FlorianR, is this correct,
                                                             // should I use Focusable or create a
                                                             // new constant?
  }
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_TIMELINE_H_
