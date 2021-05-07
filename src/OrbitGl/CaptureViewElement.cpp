// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElement.h"

#include "TimeGraph.h"
#include "Viewport.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(CaptureViewElement* parent, TimeGraph* time_graph,
                                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout)
    : viewport_(viewport), layout_(layout), time_graph_(time_graph), parent_(parent) {
  CHECK(layout != nullptr);
}

void CaptureViewElement::OnPick(int x, int y) {
  mouse_pos_last_click_ = viewport_->ScreenToWorldPos(Vec2i(x, y));
  picking_offset_ = mouse_pos_last_click_ - pos_;
  mouse_pos_cur_ = mouse_pos_last_click_;
  picked_ = true;
}

void CaptureViewElement::OnRelease() {
  picked_ = false;
  RequestUpdate();
}

void CaptureViewElement::OnDrag(int x, int y) {
  mouse_pos_cur_ = viewport_->ScreenToWorldPos(Vec2i(x, y));
  RequestUpdate();
}

void CaptureViewElement::RequestUpdate() {
  if (parent_ != nullptr) {
    parent_->RequestUpdate();
  }
}

}  // namespace orbit_gl