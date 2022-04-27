// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Button.h"

#include "AccessibleCaptureViewElement.h"

namespace orbit_gl {

Button::Button(CaptureViewElement* parent, const Viewport* viewport, const TimeGraphLayout* layout)
    : CaptureViewElement(parent, viewport, layout) {
  SetWidth(layout->GetMinButtonSize());
  SetHeight(layout->GetMinButtonSize());
}

void Button::SetHeight(float height) {
  if (height == height_) return;

  height_ = height;
  RequestUpdate();
}

void Button::SetLabel(const std::string& label) {
  if (label_ == label) return;

  label_ = label;
  RequestUpdate(RequestUpdateScope::kDraw);
}

void Button::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  SetWidth(std::max(GetWidth(), layout_->GetMinButtonSize()));
  SetHeight(std::max(GetHeight(), layout_->GetMinButtonSize()));
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Button::CreateAccessibleInterface() {
  // TODO: Should be `AccessibleButton`
  return std::make_unique<AccessibleCaptureViewElement>(this, "TODO");
}

}  // namespace orbit_gl