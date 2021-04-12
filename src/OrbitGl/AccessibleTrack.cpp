// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTrack.h"

#include <GteVector.h>

#include <algorithm>

#include "AccessibleThreadBar.h"
#include "AccessibleTimeGraph.h"
#include "CoreMath.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "TimeGraph.h"
#include "Track.h"

using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace orbit_gl {

const AccessibleInterface* AccessibleTrackTab::AccessibleChild(int index) const {
  if (index == 0) {
    return track_->GetTriangleToggle()->GetOrCreateAccessibleInterface();
  }
  return nullptr;
}

const AccessibleInterface* AccessibleTrackTab::AccessibleParent() const {
  return track_->GetOrCreateAccessibleInterface();
}

std::string AccessibleTrackTab::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_tab";
}

AccessibilityRect AccessibleTrackTab::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);

  GlCanvas* canvas = track_->GetCanvas();
  const Viewport& viewport = canvas->GetViewport();

  return AccessibilityRect(viewport.WorldToScreenWidth(layout_->GetTrackTabOffset()),
                           viewport.WorldToScreenHeight(0),
                           viewport.WorldToScreenWidth(layout_->GetTrackTabWidth()),
                           viewport.WorldToScreenHeight(layout_->GetTrackTabHeight()));
}

orbit_accessibility::AccessibilityRect AccessibleTimerPane::AccessibleLocalRect() const {
  // This implies a lot of knowledge about the structure of the parent:
  // The TimerPane is not a real control and only created to expose areas of the track to the
  // E2E tests. We know that the TimerPane is always the last child of a track.

  orbit_accessibility::AccessibilityRect parent_rect = parent_->AccessibleLocalRect();

  int parents_child_count = parent_->AccessibleChildCount();
  // We are in a TimerPane, so there are at least the "tab" and the "timers" as children.
  CHECK(parents_child_count >= 2);

  orbit_accessibility::AccessibilityRect last_child_rect =
      parent_->AccessibleChild(parent_->AccessibleChildCount() - 2)->AccessibleLocalRect();
  orbit_accessibility::AccessibilityRect result;
  result.width = parent_rect.width;
  result.top = last_child_rect.top + last_child_rect.height;
  result.height = parent_rect.height - result.top;
  return result;
}

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
    return &tab_;
  }

  const auto& children = track_->GetVisibleChildren();
  auto child_count = static_cast<int>(children.size());

  // The last child is the timer pane if it has timers.
  if (index == child_count + 1 && track_->GetVisiblePrimitiveCount() > 0) {
    return &timers_pane_;
  }

  // Are we out of bound?
  if (index < 0 || index >= child_count + 1) {
    return nullptr;
  }

  // Indexes between 1 and child_count + 1 are reserved for the actual children.
  return children[index - 1]->GetOrCreateAccessibleInterface();
}

const AccessibleInterface* AccessibleTrack::AccessibleParent() const {
  CHECK(track_ != nullptr);
  return track_->GetParent()->GetOrCreateAccessibleInterface();
}

std::string AccessibleTrack::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName();
}

AccessibilityRect AccessibleTrack::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);
  CHECK(track_->GetCanvas() != nullptr);

  GlCanvas* canvas = track_->GetCanvas();
  Vec2 pos = track_->GetPos();
  Vec2 size = track_->GetSize();

  const Viewport& viewport = canvas->GetViewport();

  Vec2i screen_pos = viewport.WorldToScreenPos(pos);
  int screen_width = viewport.WorldToScreenWidth(size[0]);
  int screen_height = viewport.WorldToScreenHeight(size[1]);

  // Adjust the coordinates to clamp the result to an on-screen rect
  // This will "cut" any part that is offscreen due to scrolling, and may result
  // in a final result with width / height of 0.

  // First: Clamp bottom
  if (screen_pos[1] + screen_height > viewport.GetScreenHeight()) {
    screen_height = std::max(0, viewport.GetScreenHeight() - static_cast<int>(screen_pos[1]));
  }
  // Second: Clamp top
  if (screen_pos[1] < 0) {
    screen_height = std::max(0, static_cast<int>(screen_pos[1]) + screen_height);
    screen_pos[1] = 0;
  }

  return AccessibilityRect(static_cast<int>(screen_pos[0]), static_cast<int>(screen_pos[1]),
                           screen_width, screen_height);
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

  if (AccessibleLocalRect().height == 0) {
    result |= State::Offscreen;
  }
  return result;
}

}  // namespace orbit_gl