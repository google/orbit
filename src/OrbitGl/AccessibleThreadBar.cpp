// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleThreadBar.h"

#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "ThreadBar.h"
#include "Track.h"

namespace orbit_gl {

int orbit_gl::AccessibleThreadBar::AccessibleChildCount() const { return 0; }

const orbit_accessibility::AccessibleInterface* AccessibleThreadBar::AccessibleParent() const {
  CHECK(thread_bar_ != nullptr);

  if (thread_bar_->GetParent() == nullptr) return nullptr;
  if (thread_bar_->GetParent()->GetAccessibleInterface() == nullptr) return nullptr;

  // This relies on knowledge about AccessibleTrack: Accessibility is split into "virtual" controls
  // that only exist as children in the accessibility tree. In particular, the track is split into
  // a tab and a content area, and the thread bar is parented underneath the content area (child 1).
  // This could be cleaned up by introducing "real" control elements for tab and content area.
  return thread_bar_->GetParent()->GetAccessibleInterface()->AccessibleChild(1);
}

std::string AccessibleThreadBar::AccessibleName() const { return thread_bar_->GetName(); }

orbit_accessibility::AccessibilityRect orbit_gl::AccessibleThreadBar::AccessibleLocalRect() const {
  CHECK(thread_bar_ != nullptr);

  Vec2 pos = thread_bar_->GetPos();
  Vec2 local_pos;

  // TODO: We should be able to specialize the GetParentMethod in subclasses.
  const auto* parent = dynamic_cast<const CaptureViewElement*>(thread_bar_->GetParent());
  // TODO(b/177350599): This could be cleaned up with clearer coordinate systems
  if (parent != nullptr) {
    Vec2 parent_pos = parent->GetPos();
    local_pos = Vec2(pos[0] - parent_pos[0], parent_pos[1] - pos[1]);
  } else {
    local_pos = pos;
  }

  GlCanvas* canvas = thread_bar_->GetCanvas();
  CHECK(canvas != nullptr);

  return orbit_accessibility::AccessibilityRect(
      canvas->WorldToScreenWidth(local_pos[0]), canvas->WorldToScreenHeight(local_pos[1]),
      canvas->WorldToScreenWidth(thread_bar_->GetSize()[0]),
      canvas->WorldToScreenHeight(thread_bar_->GetSize()[1]));
}

orbit_accessibility::AccessibilityState AccessibleThreadBar::AccessibleState() const {
  return orbit_accessibility::AccessibilityState::Focusable;
}

}  // namespace orbit_gl