// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTimeGraph.h"

#include "GlCanvas.h"
#include "TimeGraph.h"

AccessibilityRect TimeGraphAccessibility::AccessibleLocalRect() const {
  GlCanvas* canvas = time_graph_->GetCanvas();
  return AccessibilityRect(0, 0, canvas->GetWidth(), canvas->GetHeight());
}

AccessibilityState TimeGraphAccessibility::AccessibleState() const {
  return orbit_gl::AccessibilityState::Focusable;
}

int TimeGraphAccessibility::AccessibleChildCount() const {
  return time_graph_->GetVisibleTracks().size();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleChild(int index) const {
  return time_graph_->GetVisibleTracks()[index]->AccessibilityInterface();
}

const AccessibleInterface* TimeGraphAccessibility::AccessibleParent() const {
  return time_graph_->GetCanvas()->GetOrCreateAccessibleInterface();
}
