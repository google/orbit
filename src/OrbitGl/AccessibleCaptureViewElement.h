// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_

#include "CaptureViewElement.h"
#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_gl {
class AccessibleCaptureViewElement : public orbit_accessibility::AccessibleInterface {
 public:
  explicit AccessibleCaptureViewElement(const CaptureViewElement* capture_view_element)
      : capture_view_element_(capture_view_element) {
    CHECK(capture_view_element != nullptr);
  }

  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override {
    return orbit_accessibility::AccessibilityState::Normal;
  }

 private:
  const CaptureViewElement* capture_view_element_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_
