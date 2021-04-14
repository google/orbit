// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_
#define ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_

#include <string>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"

class TimeGraph;

namespace orbit_gl {

class TimeGraphAccessibility : public orbit_accessibility::AccessibleInterface {
 public:
  explicit TimeGraphAccessibility(TimeGraph* time_graph) : time_graph_(time_graph) {
    CHECK(time_graph != nullptr);
  }
  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int index) const override;
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override { return "TimeGraph"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Graphic;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  TimeGraph* time_graph_;
};

}  // namespace orbit_gl

#endif