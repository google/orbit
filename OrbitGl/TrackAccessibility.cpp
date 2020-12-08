// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackAccessibility.h"

#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"

namespace orbit_gl {

const GlA11yControlInterface* TrackContentAccessibility::AccessibleParent() const {
  return track_->AccessibilityInterface();
}

std::string TrackContentAccessibility::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_content";
}

A11yRect TrackContentAccessibility::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);
  CHECK(track_->GetTimeGraph() != nullptr);

  auto& layout = track_->GetTimeGraph()->GetLayout();
  return A11yRect(0, layout.GetTrackTabHeight(), track_->GetSize()[0], track_->GetSize()[1]);
}

A11yState TrackContentAccessibility::AccessibleState() const {
  A11yState result;
  return result;
}

const GlA11yControlInterface* TrackTabAccessibility::AccessibleParent() const {
  return track_->AccessibilityInterface();
}

std::string TrackTabAccessibility::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName() + "_tab";
}

A11yRect TrackTabAccessibility::AccessibleLocalRect() const {
  CHECK(track_ != nullptr);
  CHECK(track_->GetTimeGraph() != nullptr);

  auto& layout = track_->GetTimeGraph()->GetLayout();
  return A11yRect(layout.GetTrackTabOffset(), 0, layout.GetTrackTabWidth(),
                  layout.GetTrackTabHeight());
}

A11yState TrackTabAccessibility::AccessibleState() const {
  A11yState result;
  return result;
}

const GlA11yControlInterface* TrackAccessibility::AccessibleParent() const {
  CHECK(track_ != nullptr);
  return track_->GetTimeGraph();
}

std::string TrackAccessibility::AccessibleName() const {
  CHECK(track_ != nullptr);
  return track_->GetName();
}

A11yRect TrackAccessibility::AccessibleLocalRect() const {
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

  return A11yRect(left, top, width, height);
}

A11yState TrackAccessibility::AccessibleState() const {
  CHECK(track_ != nullptr);

  A11yState result;
  result.active = result.focusable = result.selectable = result.movable = 1;
  result.focused = track_->IsTrackSelected();
  result.selected = track_->IsTrackSelected();
  result.expandable = !track_->IsCollapsable();
  result.expanded = !track_->IsCollapsed();
  result.collapsed = track_->IsCollapsed();
  result.offscreen = AccessibleLocalRect().height == 0;
  return result;
}

}  // namespace orbit_gl