// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_ACCESSIBILITY_ACCESSIBLE_WIDGET_BRIDGE_H_
#define ORBIT_ACCESSIBILITY_ACCESSIBLE_WIDGET_BRIDGE_H_

#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_accessibility {

/*
 * Specialization of AccessibleInterface to bridge an OpenGl child element and its QtWidget
 * parent, providing a default implementation for all methods.
 */
class AccessibleWidgetBridge : public AccessibleInterface {
 public:
  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override {
    return nullptr;
  }
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override { return nullptr; }

  // The methods below are usually not used and are instead handled by the QtWidget.
  // See the implementation of OpenGlWidgetAccessible in AccessibilityAdapter.cpp!
  [[nodiscard]] std::string AccessibleName() const override { return ""; };
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Grouping;
  }
  [[nodiscard]] AccessibilityRect AccessibleRect() const override { return {}; }
  [[nodiscard]] AccessibilityState AccessibleState() const override {
    return AccessibilityState::Normal;
  }
};

}  // namespace orbit_accessibility

#endif