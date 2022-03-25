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
 * Accessibility information for the track.
 * This will return the two "virtual" controls for the title tab and the timers in addition to the
 * actual children of the track.
 * The title tab is the first child and the timers pane is the last child.
 */
class AccessibleTrack : public AccessibleCaptureViewElement {
 public:
  AccessibleTrack(Track* track, const TimeGraphLayout* layout);

  [[nodiscard]] int AccessibleChildCount() const override;
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int index) const override;

  [[nodiscard]] orbit_accessibility::AccessibilityState AccessibleState() const override;

 private:
  Track* track_;
  std::unique_ptr<CaptureViewElement> fake_tab_;
  std::unique_ptr<CaptureViewElement> fake_timers_pane_;
};

}  // namespace orbit_gl

#endif