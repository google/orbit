// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_THREAD_BAR_H_
#define ORBIT_GL_ACCESSIBLE_THREAD_BAR_H_

#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_gl {

class ThreadBar;

class AccessibleThreadBar : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleThreadBar(const ThreadBar* thread_bar) : thread_bar_(thread_bar) {}

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override {
    return nullptr;
  }
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Pane;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  const ThreadBar* thread_bar_;
};

}  // namespace orbit_gl

#endif