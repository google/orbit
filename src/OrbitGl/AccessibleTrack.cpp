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

class FakeTrackTab : public CaptureViewElement {
 public:
  explicit FakeTrackTab(Track* track, const TimeGraphLayout* layout)
      : CaptureViewElement(track, track->GetViewport(), layout), track_(track) {
    // Compute and set the size (which would usually be done by the parent). As the position may
    // change, we override the GetPos() function.
    SetWidth(layout_->GetTrackTabWidth());
  }

  [[nodiscard]] std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() override {
    return std::make_unique<AccessibleCaptureViewElement>(
        this, track_->GetName() + "_tab", orbit_accessibility::AccessibilityRole::PageTab);
  }

  [[nodiscard]] float GetHeight() const override { return layout_->GetTrackTabHeight(); }

  [[nodiscard]] Vec2 GetPos() const override {
    Vec2 track_pos = track_->GetPos();
    Vec2 pos = track_pos + Vec2(layout_->GetTrackTabOffset(), 0);
    return pos;
  }

 private:
  Track* track_;
};

class FakeTimerPane : public CaptureViewElement {
 public:
  explicit FakeTimerPane(Track* track, const TimeGraphLayout* layout, CaptureViewElement* track_tab)
      : CaptureViewElement(track, track->GetViewport(), layout),
        track_(track),
        track_tab_(track_tab) {
    SetWidth(track->GetWidth());
  }

  std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() override {
    return std::make_unique<AccessibleCaptureViewElement>(this, "Timers");
  }

  [[nodiscard]] Vec2 GetPos() const override {
    std::vector<CaptureViewElement*> track_children = track_->GetNonHiddenChildren();

    // We are looking for the track's child that is right above the timer pane.
    CaptureViewElement* predecessor = track_tab_;

    if (!track_children.empty()) {
      predecessor = track_children[track_children.size() - 1];
    }

    Vec2 pos{track_->GetPos()[0], predecessor->GetPos()[1] + predecessor->GetSize()[1]};
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
  CaptureViewElement* track_tab_;
};

}  //  namespace

AccessibleTrack::AccessibleTrack(Track* track, const TimeGraphLayout* layout)
    : AccessibleCaptureViewElement(track, track->GetName(),
                                   orbit_accessibility::AccessibilityRole::Grouping),
      track_(track),
      fake_tab_(std::make_unique<FakeTrackTab>(track, layout)),
      fake_timers_pane_(std::make_unique<FakeTimerPane>(track, layout, fake_tab_.get())) {}

int AccessibleTrack::AccessibleChildCount() const {
  ORBIT_CHECK(track_ != nullptr);

  // Only expose the "Timer" pane if any timers were rendered in the visible field
  if (track_->GetVisiblePrimitiveCount() > 0) {
    return static_cast<int>(track_->GetNonHiddenChildren().size()) + 2;
  }

  return static_cast<int>(track_->GetNonHiddenChildren().size()) + 1;
}

const AccessibleInterface* AccessibleTrack::AccessibleChild(int index) const {
  ORBIT_CHECK(track_ != nullptr);

  // The first child is the "virtual" tab.
  if (index == 0) {
    return fake_tab_->GetOrCreateAccessibleInterface();
  }

  const auto& children = track_->GetNonHiddenChildren();
  auto child_count = static_cast<int>(children.size());

  // The last child is the timer pane if it has timers.
  if (index == child_count + 1 && track_->GetVisiblePrimitiveCount() > 0) {
    return fake_timers_pane_->GetOrCreateAccessibleInterface();
  }

  // Are we out of bound?
  if (index < 0 || index >= child_count + 1) {
    return nullptr;
  }

  // Indexes between 1 and child_count + 1 are reserved for the actual children.
  return children[index - 1]->GetOrCreateAccessibleInterface();
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