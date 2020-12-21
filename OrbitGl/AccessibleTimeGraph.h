// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_
#define ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"

class TimeGraph;

using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityRole;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

class TimeGraphAccessibility : public AccessibleInterface {
 public:
  TimeGraphAccessibility(TimeGraph* time_graph) : time_graph_(time_graph) {
    CHECK(time_graph != nullptr);
  }
  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override { return "TimeGraph"; }
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Graphic;
  }
  [[nodiscard]] AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] AccessibilityState AccessibleState() const override;

 private:
  TimeGraph* time_graph_;
};

#endif