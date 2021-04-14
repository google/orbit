// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_TRACK_H_
#define ORBIT_GL_ACCESSIBLE_TRACK_H_

#include <string>

#include "AccessibleCaptureViewElement.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "TimeGraphLayout.h"

class Track;

namespace orbit_gl {

/*
 * Accessibility implementation for the track tab.
 * This is a "virtual" control and will be generated as a child of AccessibleTrack to make the track
 * tab clickable.
 */
class AccessibleTrackTab : public AccessibleCaptureViewElement {
 public:
  AccessibleTrackTab(CaptureViewElement* fake_track_tab, Track* track);

  [[nodiscard]] int AccessibleChildCount() const override { return 1; }
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::PageTab;
  }

 private:
  Track* track_;
};

class AccessibleTimerPane : public AccessibleCaptureViewElement {
 public:
  explicit AccessibleTimerPane(CaptureViewElement* fake_timer_pane);

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* AccessibleChild(
      int /*index*/) const override {
    return nullptr;
  }

  [[nodiscard]] std::string AccessibleName() const override { return "Timers"; }
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Pane;
  }
};

/*
 * Accessibility information for the track.
 * This will return the two "virtual" controls for the title tab and the timers in addition to the
 * actual children of the track.
 * The title tab is the first child and the timers pane is the last child.
 */
class AccessibleTrack : public AccessibleCaptureViewElement {
 public:
  AccessibleTrack(Track* track, TimeGraphLayout* layout);

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int index) const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_accessibility::AccessibilityRole AccessibleRole() const override {
    return orbit_accessibility::AccessibilityRole::Grouping;
  }
  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  std::unique_ptr<CaptureViewElement> fake_tab_;
  std::unique_ptr<CaptureViewElement> fake_timers_pane_;
};

}  // namespace orbit_gl

#endif