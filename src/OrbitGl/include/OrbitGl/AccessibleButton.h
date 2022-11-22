// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_BUTTON_H_
#define ORBIT_GL_ACCESSIBLE_BUTTON_H_

#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/Button.h"

namespace orbit_gl {
class AccessibleButton : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleButton(const Button* button)
      : AccessibleCaptureViewElement(button, "Button",
                                     orbit_accessibility::AccessibilityRole::Button),
        button_(button) {
    ORBIT_CHECK(button_ != nullptr);
  }
  [[nodiscard]] std::string AccessibleName() const override { return button_->GetName(); }

 private:
  const Button* button_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_CAPTURE_VIEW_ELEMENT_H_
