// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackAccessibility.h"

#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"

namespace orbit_gl {

using orbit_accessibility::AccessibleInterface;

const AccessibleInterface* AccessibleTrackContent::AccessibleParent() const {
  return track_->AccessibilityInterface();
}

std::string AccessibleTrackContent::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_content";
}

AccessibilityRect AccessibleTrackContent::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);
  CHECK(track_->GetTimeGraph() != nullptr);

  auto& layout = track_->GetTimeGraph()->GetLayout();
  return AccessibilityRect(0, static_cast<int>(layout.GetTrackTabHeight()),
                           static_cast<int>(track_->GetSize()[0]),
                           static_cast<int>(track_->GetSize()[1]));
}

AccessibilityState AccessibleTrackContent::AccessibleState() const {
  AccessibilityState result;
  return result;
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
  CHECK(track_->GetTimeGraph() != nullptr);

  auto& layout = track_->GetTimeGraph()->GetLayout();
  return AccessibilityRect(static_cast<int>(layout.GetTrackTabOffset()), 0,
                           static_cast<int>(layout.GetTrackTabWidth()),
                           static_cast<int>(layout.GetTrackTabHeight()));
}

AccessibilityState AccessibleTrackTab::AccessibleState() const {
  AccessibilityState result;
  return result;
}

const AccessibleInterface* AccessibleTrack::AccessibleParent() const {
  CHECK(track_ != nullptr);
  return track_->GetTimeGraph()->Accessibility();
}

std::string AccessibleTrack::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName();
}

AccessibilityRect AccessibleTrack::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);
  CHECK(track_->GetTimeGraph() != nullptr);
  CHECK(track_->GetTimeGraph()->GetCanvas() != nullptr);

  auto canvas = track_->GetTimeGraph()->GetCanvas();
  auto pos = track_->GetPos();
  auto size = track_->GetSize();
  auto layout = track_->GetTimeGraph()->GetLayout();

  int top = static_cast<int>(
      std::max(-pos[1] + canvas->GetWorldTopLeftY() - layout.GetTrackTabHeight(), 0.0f));
  int left = static_cast<int>(pos[0]);
  int width = static_cast<int>(size[0]);
  int height =
      std::max(std::min(static_cast<int>(size[1] + layout.GetTrackTabHeight() - std::min(top, 0)),
                        canvas->GetHeight() - top),
               0);

  return AccessibilityRect(left, top, width, height);
}

AccessibilityState AccessibleTrack::AccessibleState() const {
  CHECK(track_ != nullptr);

  using State = orbit_accessibility::AccessibilityState;

  State result = State::Normal;
  result |= State::Focusable | State::Movable;
  if (track_->IsTrackSelected()) {
    result |= State::Focused;
  }
  if (track_->IsCollapsable()) {
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