// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AccessibleThreadBar.h"

#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "ThreadBar.h"
#include "Track.h"
#include "Viewport.h"

namespace orbit_gl {

AccessibleThreadBar::AccessibleThreadBar(const ThreadBar* thread_bar)
    : AccessibleCaptureViewElement(thread_bar), thread_bar_(thread_bar) {}

int orbit_gl::AccessibleThreadBar::AccessibleChildCount() const { return 0; }

std::string AccessibleThreadBar::AccessibleName() const { return thread_bar_->GetName(); }

orbit_accessibility::AccessibilityState AccessibleThreadBar::AccessibleState() const {
  return orbit_accessibility::AccessibilityState::Focusable;
}

}  // namespace orbit_gl