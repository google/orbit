// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_

#include <string>
#include <utility>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CaptureViewElement.h"

namespace orbit_gl {
class AccessibleCaptureViewElement : public orbit_accessibility::AccessibleInterface {
 public:
  explicit AccessibleCaptureViewElement(const CaptureViewElement* capture_view_element,
                                        std::string accessible_name,
                                        orbit_accessibility::AccessibilityRole accessible_role =
                                            orbit_accessibility::AccessibilityRole::Pane,
                                        orbit_accessibility::AccessibilityState accessible_state =
                                            orbit_accessibility::AccessibilityState::Normal)
      : accessible_name_(std::move(accessible_name)),
        accessible_role_(accessible_role),
        accessible_state_(accessible_state),
        capture_view_element_(capture_view_element) {
    ORBIT_CHECK(capture_view_element != nullptr);
  }

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int index) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override { return accessible_name_; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return accessible_role_;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override {
    return accessible_state_;
  }

 private:
  const std::string accessible_name_;
  const orbit_accessibility::AccessibilityRole accessible_role_;
  const orbit_accessibility::AccessibilityState accessible_state_;
  const CaptureViewElement* capture_view_element_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_
