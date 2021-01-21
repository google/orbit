// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackAccessibility.h"

#include <GteVector.h>

#include <algorithm>

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

int AccessibleTrackContent::AccessibleChildCount() const { return 0; }

const AccessibleInterface* AccessibleTrackContent::AccessibleChild(int /*index*/) const {
  return nullptr;
}

const AccessibleInterface* AccessibleTrackContent::AccessibleParent() const {
  return track_->AccessibilityInterface();
}

std::string AccessibleTrackContent::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_content";
}

AccessibilityRect AccessibleTrackContent::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);

  GlCanvas* canvas = track_->GetCanvas();

  return AccessibilityRect(canvas->WorldToScreenWidth(0),
                           canvas->WorldToScreenHeight(layout_->GetTrackTabHeight()),
                           canvas->WorldToScreenWidth(track_->GetSize()[0]),
                           canvas->WorldToScreenHeight(track_->GetSize()[1]));
}

AccessibilityState AccessibleTrackContent::AccessibleState() const {
  return AccessibilityState::Normal;
}

int AccessibleTrackTab::AccessibleChildCount() const { return 0; }

const AccessibleInterface* AccessibleTrackTab::AccessibleChild(int /*index*/) const {
  return nullptr;
}

const AccessibleInterface* AccessibleTrackTab::AccessibleParent() const {
  return track_->AccessibilityInterface();
}

std::string AccessibleTrackTab::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_tab";
}

AccessibilityRect AccessibleTrackTab::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);

  GlCanvas* canvas = track_->GetCanvas();

  return AccessibilityRect(canvas->WorldToScreenWidth(layout_->GetTrackTabOffset()),
                           canvas->WorldToScreenHeight(0),
                           canvas->WorldToScreenWidth(layout_->GetTrackTabWidth()),
                           canvas->WorldToScreenHeight(layout_->GetTrackTabHeight()));
}

AccessibilityState AccessibleTrackTab::AccessibleState() const {
  return AccessibilityState::Normal;
}

int AccessibleTrack::AccessibleChildCount() const { return 2; }

const AccessibleInterface* AccessibleTrack::AccessibleChild(int index) const {
  if (index == 0) {
    return &tab_;
  }
  return &content_;
}

const AccessibleInterface* AccessibleTrack::AccessibleParent() const {
  CHECK(track_ != nullptr);
  return track_->GetTimeGraph()->GetOrCreateAccessibleInterface();
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
  pos[1] += layout_->GetTrackTabHeight();
  Vec2 size = track_->GetSize();
  size[1] += layout_->GetTrackTabHeight();

  Vec2 screen_pos = canvas->WorldToScreen(pos);
  int screen_width = canvas->WorldToScreenWidth(size[0]);
  int screen_height = canvas->WorldToScreenHeight(size[1]);

  // Adjust the coordinates to clamp the result to an on-screen rect
  // This will "cut" any part that is offscreen due to scrolling, and may result
  // in a final result with width / height of 0.

  // First: Clamp bottom
  if (screen_pos[1] + screen_height > canvas->GetHeight()) {
    screen_height = std::max(0, canvas->GetHeight() - static_cast<int>(screen_pos[1]));
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

  using State = orbit_accessibility::AccessibilityState;

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