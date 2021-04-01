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
using orbit_accessibility::AccessibilityRole;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace {
class AccessibleTimerPane : public AccessibleInterface {
 public:
  AccessibleTimerPane(orbit_gl::AccessibleTrackContent* parent) : parent_(parent) {}

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override {
    return nullptr;
  }
  [[nodiscard]] const AccessibleInterface* AccessibleParent() const override { return parent_; }

  [[nodiscard]] std::string AccessibleName() const override { return "Timers"; }
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Pane;
  }
  [[nodiscard]] AccessibilityRect AccessibleLocalRect() const override {
    // This implies a lot of knowledge about the structure of the parent:
    // The TimerPane is not a real control and only created to expose areas of the track to the
    // E2E tests. We know that the TimerPane is always the last child of the content pane of a
    // track, and that the content pane does not have any margins / paddings.
    AccessibilityRect parent_rect = parent_->AccessibleLocalRect();

    if (parent_->AccessibleChildCount() <= 1) {
      return AccessibilityRect(0, 0, parent_rect.width, parent_rect.height);
    }

    AccessibilityRect last_child_rect =
        parent_->AccessibleChild(parent_->AccessibleChildCount() - 2)->AccessibleLocalRect();
    AccessibilityRect result;
    result.width = parent_rect.width;
    result.top = last_child_rect.top + last_child_rect.height;
    result.height = parent_rect.height - result.top;
    return result;
  }

  [[nodiscard]] AccessibilityState AccessibleState() const override {
    return AccessibilityState::Normal;
  }

 private:
  orbit_gl::AccessibleTrackContent* parent_;
};
}  // namespace

namespace orbit_gl {

AccessibleTrackContent::AccessibleTrackContent(Track* track, TimeGraphLayout* layout)
    : track_(track), layout_(layout), timer_pane_(new AccessibleTimerPane(this)) {}

int AccessibleTrackContent::AccessibleChildCount() const {
  int result = static_cast<int>(track_->GetVisibleChildren().size());
  if (track_->GetVisiblePrimitiveCount() > 0) {
    // Only expose the "Timer" pane if any timers were rendered in the visible field
    result += 1;
  }

  return result;
}

const AccessibleInterface* AccessibleTrackContent::AccessibleChild(int index) const {
  if (index < 0) {
    return nullptr;
  }

  auto children = track_->GetVisibleChildren();
  if (static_cast<size_t>(index) == children.size()) {
    return timer_pane_.get();
  }
  if (static_cast<size_t>(index) > children.size()) {
    return nullptr;
  }
  return children[index]->GetOrCreateAccessibleInterface();
}

const AccessibleInterface* AccessibleTrackContent::AccessibleParent() const {
  return track_->GetOrCreateAccessibleInterface();
}

std::string AccessibleTrackContent::AccessibleName() const { return "Content"; }

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

int AccessibleTrackTab::AccessibleChildCount() const { return 1; }

const AccessibleInterface* AccessibleTrackTab::AccessibleChild(int /*index*/) const {
  return track_->GetTriangleToggle()->GetOrCreateAccessibleInterface();
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