// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TRACK_CONTAINER_H_
#define ORBIT_GL_ACCESSIBLE_TRACK_CONTAINER_H_

#include <string>

#include "AccessibleCaptureViewElement.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "TrackContainer.h"

namespace orbit_gl {

// Accessibility information for TrackContainer.
class AccessibleTrackContainer : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTrackContainer(TrackContainer* track_container)
      : AccessibleCaptureViewElement(track_container), track_container_(track_container){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int index) const override;

  [[nodiscard]] std::string AccessibleName() const override { return "Track Container"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Grouping;  // TODO: FlorianR, is this correct?
  }
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  TrackContainer* track_container_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_ACCESSIBLE_TRACK_CONTAINER_H_
