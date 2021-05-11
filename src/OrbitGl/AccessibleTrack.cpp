// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTrack.h"

#include <GteVector.h>

#include <algorithm>

#include "AccessibleThreadBar.h"
#include "AccessibleTimeGraph.h"
#include "CoreMath.h"
#include "OrbitBase/Logging.h"
#include "TimeGraph.h"
#include "Track.h"
#include "Viewport.h"

using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace orbit_gl {

namespace {

class FakeTrackTab : public CaptureViewElement {
 public:
  explicit FakeTrackTab(Track* track, TimeGraphLayout* layout)
      : CaptureViewElement(track, track->GetTimeGraph(), track->GetViewport(), layout),
        track_(track) {
    // Compute and set the size (which would usually be done by the parent). As the position may
    // change, we override the GetPos() function.
    SetSize(layout_->GetTrackTabWidth(), layout_->GetTrackTabHeight());
  }

  [[nodiscard]] std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() override {
    return std::make_unique<AccessibleTrackTab>(this, track_);
  }

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
  explicit FakeTimerPane(Track* track, TimeGraphLayout* layout, CaptureViewElement* track_tab)
      : CaptureViewElement(track, track->GetTimeGraph(), track->GetViewport(), layout),
        track_(track),
        track_tab_(track_tab) {}

  std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() override {
    return std::make_unique<AccessibleTimerPane>(this);
  }

  [[nodiscard]] Vec2 GetPos() const override {
    std::vector<CaptureViewElement*> track_children = track_->GetVisibleChildren();

    // We are looking for the track's child that is right above the timer pane.
    CaptureViewElement* predecessor = track_tab_;

    if (!track_children.empty()) {
      predecessor = track_children[track_children.size() - 1];
    }

    Vec2 pos{track_->GetPos()[0], predecessor->GetPos()[1] - predecessor->GetSize()[1]};
    return pos;
  }

  [[nodiscard]] Vec2 GetSize() const override {
    Vec2 size = track_->GetSize();
    float track_header_height = track_->GetPos()[1] - GetPos()[1];
    size[1] -= track_header_height;
    return size;
  }

 private:
  Track* track_;
  CaptureViewElement* track_tab_;
};

}  //  namespace

AccessibleTrackTab::AccessibleTrackTab(CaptureViewElement* fake_track_tab, Track* track)
    : AccessibleCaptureViewElement(fake_track_tab), track_(track) {}

const AccessibleInterface* AccessibleTrackTab::AccessibleChild(int index) const {
  if (index == 0) {
    return track_->GetTriangleToggle()->GetOrCreateAccessibleInterface();
  }
  return nullptr;
}

std::string AccessibleTrackTab::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_tab";
}

AccessibleTimerPane::AccessibleTimerPane(CaptureViewElement* fake_timer_pane)
    : AccessibleCaptureViewElement(fake_timer_pane) {}

AccessibleTrack::AccessibleTrack(Track* track, TimeGraphLayout* layout)
    : AccessibleCaptureViewElement(track),
      track_(track),
      fake_tab_(std::make_unique<FakeTrackTab>(track, layout)),
      fake_timers_pane_(std::make_unique<FakeTimerPane>(track, layout, fake_tab_.get())) {}

int AccessibleTrack::AccessibleChildCount() const {
  CHECK(track_ != nullptr);

  // Only expose the "Timer" pane if any timers were rendered in the visible field
  if (track_->GetVisiblePrimitiveCount() > 0) {
    return static_cast<int>(track_->GetVisibleChildren().size()) + 2;
  }

  return static_cast<int>(track_->GetVisibleChildren().size()) + 1;
}

const AccessibleInterface* AccessibleTrack::AccessibleChild(int index) const {
  CHECK(track_ != nullptr);

  // The first child is the "virtual" tab.
  if (index == 0) {
    return fake_tab_->GetOrCreateAccessibleInterface();
  }

  const auto& children = track_->GetVisibleChildren();
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

std::string AccessibleTrack::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName();
}

AccessibilityState AccessibleTrack::AccessibleState() const {
  CHECK(track_ != nullptr);

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