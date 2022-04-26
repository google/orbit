// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Button.h"

#include "AccessibleCaptureViewElement.h"

namespace orbit_gl {

Button::Button(CaptureViewElement* parent, const Viewport* viewport, const TimeGraphLayout* layout)
    : CaptureViewElement(parent, viewport, layout) {}

void Button::SetHeight(float height) {
  if (height == height_) return;

  height_ = height;
  RequestUpdate();
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Button::CreateAccessibleInterface() {
  // TODO: Should be `AccessibleButton`
  return std::make_unique<AccessibleCaptureViewElement>(this, "TODO");
}

}  // namespace orbit_gl