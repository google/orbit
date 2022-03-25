// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTrack.h"

#include <GteVector.h>

#include <algorithm>

#include "AccessibleTimeGraph.h"
#include "CoreMath.h"
#include "OrbitBase/Logging.h"
#include "Track.h"
#include "Viewport.h"

using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace orbit_gl {

namespace {

class FakeTimerPane : public CaptureViewElement {
 public:
  explicit FakeTimerPane(Track* track, const TimeGraphLayout* layout)
      : CaptureViewElement(track, track->GetViewport(), layout), track_(track) {
    SetWidth(track->GetWidth());
  }

  std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() override {
    return std::make_unique<AccessibleCaptureViewElement>(this, "Timers");
  }

  [[nodiscard]] Vec2 GetPos() const override {
    CaptureViewElement* track_tab = track_->GetAllChildren()[0];
    Vec2 pos{track_->GetPos()[0], track_tab->GetPos()[1] + track_tab->GetHeight()};
    return pos;
  }

  [[nodiscard]] float GetHeight() const override {
    float height = track_->GetHeight();
    float track_header_height = GetPos()[1] - track_->GetPos()[1];
    height -= track_header_height;
    return height;
  }

 private:
  Track* track_;
};

}  //  namespace

AccessibleTrack::AccessibleTrack(Track* track, const TimeGraphLayout* layout)
    : AccessibleCaptureViewElement(track, track->GetName(),
                                   orbit_accessibility::AccessibilityRole::Grouping),
      track_(track),
      fake_timers_pane_(std::make_unique<FakeTimerPane>(track, layout)) {}

int AccessibleTrack::AccessibleChildCount() const {
  ORBIT_CHECK(track_ != nullptr);

  // If any timers were rendered, report an additional element. The accessibility interface
  // simulates a "FakeTimerPane" to group all the timers together.
  if (track_->GetVisiblePrimitiveCount() > 0) {
    return static_cast<int>(track_->GetNonHiddenChildren().size()) + 1;
  }

  return static_cast<int>(track_->GetNonHiddenChildren().size());
}

const AccessibleInterface* AccessibleTrack::AccessibleChild(int index) const {
  ORBIT_CHECK(track_ != nullptr);

  const auto& children = track_->GetNonHiddenChildren();
  auto child_count = static_cast<int>(children.size());

  // The last child is the timer pane if it has timers.
  if (index == child_count && track_->GetVisiblePrimitiveCount() > 0) {
    return fake_timers_pane_->GetOrCreateAccessibleInterface();
  }

  // Are we out of bounds?
  if (index < 0 || index > child_count) {
    return nullptr;
  }

  // Indexes between 0 and child_count are reserved for the actual children.
  return children[index]->GetOrCreateAccessibleInterface();
}

AccessibilityState AccessibleTrack::AccessibleState() const {
  ORBIT_CHECK(track_ != nullptr);

  using State = AccessibilityState;

  State result = State::Normal | State::Focusable | State::Movable;
  if (track_->IsTrackSelected()) {
    result |= State::Focused;
  }
  if (track_->IsCollapsible()) {
    result |= State::Expandable;
    if (track_->IsCollapsed()) {
      result |= State::Collapsed;
    } else {
      result |= State::Expanded;
    }
  }

  if (AccessibleRect().height == 0) {
    result |= State::Offscreen;
  }
  return result;
}

}  // namespace orbit_gl