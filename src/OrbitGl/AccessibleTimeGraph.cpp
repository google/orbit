// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTimeGraph.h"

#include <vector>

#include "AccessibleTrack.h"
#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"
#include "TrackManager.h"

using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

AccessibilityRect TimeGraphAccessibility::AccessibleLocalRect() const {
  GlCanvas* canvas = time_graph_->GetCanvas();
  return AccessibilityRect(0, 0, canvas->GetViewport().GetWidth(),
                           canvas->GetViewport().GetHeight());
}

AccessibilityState TimeGraphAccessibility::AccessibleState() const {
  return AccessibilityState::Focusable;
}

int TimeGraphAccessibility::AccessibleChildCount() const {
  return time_graph_->GetTrackManager()->GetVisibleTracks().size();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleChild(int index) const {
  return time_graph_->GetTrackManager()
      ->GetVisibleTracks()[index]
      ->GetOrCreateAccessibleInterface();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleParent() const {
  return time_graph_->GetCanvas()->GetOrCreateAccessibleInterface();
}
