// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TRACK_H_
#define ORBIT_GL_ACCESSIBLE_TRACK_H_

#include <string>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "TimeGraphLayout.h"

class Track;

namespace orbit_gl {

/*
 * Accessibility implementation for the track content.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to split the
 * areas for the track title and the content.
 */
class AccessibleTrackContent : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleTrackContent(Track* track, TimeGraphLayout* layout) : track_(track), layout_(layout){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Grouping;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
};

/*
 * Accessibility implementation for the track tab.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to make the track
 * tab clickable.
 */
class AccessibleTrackTab : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleTrackTab(Track* track, TimeGraphLayout* layout) : track_(track), layout_(layout){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::PageTab;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
};

/*
 * Accessibility information for the track.
 * This will return two "virtual" children to split the track tab and its content.
 */
class AccessibleTrack : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleTrack(Track* track, TimeGraphLayout* layout)
      : track_(track), layout_(layout), content_(track_, layout_), tab_(track_, layout_){};

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int index) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Grouping;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  TimeGraphLayout* layout_;
  AccessibleTrackContent content_;
  AccessibleTrackTab tab_;
};

}  // namespace orbit_gl

#endif