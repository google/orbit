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
 * Accessibility implementation for the track tab.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to make the track
 * tab clickable.
 */
class AccessibleTrackTab : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleTrackTab(Track* track, TimeGraphLayout* layout) : track_(track), layout_(layout){};

  [[nodiscard]] int AccessibleChildCount() const override { return 1; }
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override;
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::PageTab;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override {
    return orbit_accessibility::AccessibilityState::Normal;
  }

 private:
  Track* track_;
  TimeGraphLayout* layout_;
};

class AccessibleTimerPane : public orbit_accessibility::AccessibleInterface {
 public:
  explicit AccessibleTimerPane(orbit_accessibility::AccessibleInterface* parent)
      : parent_(parent) {}

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int /*index*/) const override {
    return nullptr;
  }
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleParent() const override {
    return parent_;
  }

  [[nodiscard]] std::string AccessibleName() const override { return "Timers"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Pane;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityRect AccessibleLocalRect() const override;

  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override {
    return orbit_accessibility::AccessibilityState::Normal;
  }

 private:
  orbit_accessibility::AccessibleInterface* parent_;
};

/*
 * Accessibility information for the track.
 * This will return the two "virtual" controls for the title tab and the timers in addition to the
 * actual children of the track.
 * The title tab is the first child and the timers pane is the last child.
 */
class AccessibleTrack : public orbit_accessibility::AccessibleInterface {
 public:
  AccessibleTrack(Track* track, TimeGraphLayout* layout)
      : track_(track), layout_(layout), tab_(track_, layout_), timers_pane_(this){};

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
  AccessibleTrackTab tab_;
  AccessibleTimerPane timers_pane_;
};

}  // namespace orbit_gl

#endif