// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_ACCESSIBILITY_H_
#define ORBIT_GL_TRACK_ACCESSIBILITY_H_

#include <string>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "TimeGraphLayout.h"

class Track;

namespace orbit_gl {
using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityRole;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

/*
 * Accessibility implementation for the track content.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to split the
 * areas for the track title and the content.
 */
class AccessibleTrackContent : public AccessibleInterface {
 public:
  AccessibleTrackContent(Track* track, TimeGraphLayout* layout) : track_(track), layout_(layout){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Grouping;
  }
  [[nodiscard]] AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
};

/*
 * Accessibility implementation for the track tab.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to make the track
 * tab clickable.
 */
class AccessibleTrackTab : public AccessibleInterface {
 public:
  AccessibleTrackTab(Track* track, TimeGraphLayout* layout) : track_(track), layout_(layout){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::PageTab;
  }
  [[nodiscard]] AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
};

/*
 * Accessibility information for the track.
 * This will return two "virtual" children to split the track tab and its content.
 */
class AccessibleTrack : public AccessibleInterface {
 public:
  AccessibleTrack(Track* track, TimeGraphLayout* layout)
      : track_(track), layout_(layout), content_(track_, layout_), tab_(track_, layout_){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int index) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return orbit_gl::AccessibilityRole::Grouping;
  }
  [[nodiscard]] AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
  AccessibleTrackContent content_;
  AccessibleTrackTab tab_;
};

}  // namespace orbit_gl

#endif