// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AccessibleTrack.h"

#include <GteVector.h>

#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Track.h"

using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace orbit_gl {

namespace {

// TODO (b/185854980): Remove the fake elements.
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
    // The element is positioned after the last visible child. We can safely assume there's always
    // at least one child due to the track header.
    CaptureViewElement* last_child = *track_->GetNonHiddenChildren().rbegin();
    float pos_y = last_child->GetPos()[1] + last_child->GetHeight();

    if (track_->GetNonHiddenChildren().size() == 1) {
      // If there's only one child, the track only has timers and a header. In this case add the
      // content margin
      pos_y += layout_->GetTrackContentTopMargin();
    } else {
      // Otherwise, it's a thread track and we need to include the space between panes.
      // This is really hacky and will go away once this class vanishes, see the TODO on top.
      pos_y += layout_->GetSpaceBetweenThreadPanes();
    }
    Vec2 pos{track_->GetPos()[0], pos_y};
    return pos;
  }

  [[nodiscard]] float GetHeight() const override {
    float height = track_->GetHeight();
    float track_header_height = GetPos()[1] - track_->GetPos()[1];
    height -= track_header_height;
    height -= layout_->GetTrackContentBottomMargin();
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
  // TODO (b/185854980): Remove the fake elements.
  if (track_->GetVisiblePrimitiveCount() > 0) {
    return static_cast<int>(track_->GetNonHiddenChildren().size()) + 1;
  }

  return static_cast<int>(track_->GetNonHiddenChildren().size());
}

// TODO (b/185854980): Remove the fake elements.
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