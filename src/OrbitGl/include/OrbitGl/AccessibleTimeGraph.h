// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_
#define ORBIT_GL_ACCESSIBLE_TIME_GRAPH_H_

#include <string>

#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/TimeGraph.h"

namespace orbit_gl {

class AccessibleTimeGraph : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTimeGraph(TimeGraph* time_graph)
      : AccessibleCaptureViewElement(time_graph, "TimeGraph",
                                     orbit_accessibility::AccessibilityRole::Graphic,
                                     orbit_accessibility::AccessibilityState::Focusable),
        time_graph_(time_graph) {}
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleParent() const override {
    return time_graph_->GetAccessibleParent()->GetOrCreateAccessibleInterface();
  }

 private:
  TimeGraph* time_graph_;
};

}  // namespace orbit_gl

#endif