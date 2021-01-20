// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTimeGraph.h"

#include <vector>

#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"
#include "TrackAccessibility.h"
#include "TrackManager.h"

using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

AccessibilityRect TimeGraphAccessibility::AccessibleLocalRect() const {
  GlCanvas* canvas = time_graph_->GetCanvas();
  return AccessibilityRect(0, 0, canvas->GetWidth(), canvas->GetHeight());
}

AccessibilityState TimeGraphAccessibility::AccessibleState() const {
  return orbit_gl::AccessibilityState::Focusable;
}

int TimeGraphAccessibility::AccessibleChildCount() const {
  return time_graph_->GetTrackManager()->GetVisibleTracks().size();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleChild(int index) const {
  return time_graph_->GetTrackManager()->GetVisibleTracks()[index]->AccessibilityInterface();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleParent() const {
  return time_graph_->GetCanvas()->GetOrCreateAccessibleInterface();
}
