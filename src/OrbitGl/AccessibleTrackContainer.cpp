// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleTrackContainer.h"

#include <vector>

#include "AccessibleInterfaceProvider.h"
#include "AccessibleTrack.h"
#include "Track.h"
#include "TrackContainer.h"
#include "TrackManager.h"
#include "Viewport.h"

using orbit_accessibility::AccessibilityRect;
using orbit_accessibility::AccessibilityState;
using orbit_accessibility::AccessibleInterface;

namespace orbit_gl {

AccessibilityState AccessibleTrackContainer::AccessibleState() const {
  return AccessibilityState::Focusable;
}

int AccessibleTrackContainer::AccessibleChildCount() const {
  return track_container_->GetTrackManager()->GetVisibleTracks().size();
}

const AccessibleInterface* AccessibleTrackContainer::AccessibleChild(int index) const {
  return track_container_->GetTrackManager()
      ->GetVisibleTracks()[index]
      ->GetOrCreateAccessibleInterface();
}

}  // namespace orbit_gl