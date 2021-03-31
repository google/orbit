// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElement.h"

#include "GlCanvas.h"
#include "TimeGraph.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(CaptureViewElement* parent, TimeGraph* time_graph,
                                       TimeGraphLayout* layout)
    : parent_(parent), layout_(layout), time_graph_(time_graph) {
  CHECK(layout != nullptr);
}

void CaptureViewElement::OnPick(int x, int y) {
  mouse_pos_last_click_ = canvas_->GetViewport().ScreenToWorldPos(Vec2i(x, y));
  picking_offset_ = mouse_pos_last_click_ - pos_;
  mouse_pos_cur_ = mouse_pos_last_click_;
  picked_ = true;
}

void CaptureViewElement::OnRelease() {
  picked_ = false;
  time_graph_->RequestUpdatePrimitives();
}

void CaptureViewElement::OnDrag(int x, int y) {
  mouse_pos_cur_ = canvas_->GetViewport().ScreenToWorldPos(Vec2i(x, y));
  time_graph_->RequestUpdatePrimitives();
}

orbit_accessibility::AccessibleInterface* CaptureViewElement::GetOrCreateAccessibleInterface() {
  if (accessibility_ == nullptr) {
    accessibility_ = CreateAccessibleInterface();
  }
  return accessibility_.get();
}

}  // namespace orbit_gl