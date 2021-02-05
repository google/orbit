// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElement.h"

#include "GlCanvas.h"
#include "TimeGraph.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(TimeGraph* time_graph) : time_graph_(time_graph) {}

void CaptureViewElement::OnPick(int x, int y) {
  canvas_->ScreenToWorld(x, y, mouse_pos_last_click_[0], mouse_pos_last_click_[1]);
  picking_offset_ = mouse_pos_last_click_ - pos_;
  mouse_pos_cur_ = mouse_pos_last_click_;
  picked_ = true;
}

void CaptureViewElement::OnRelease() {
  picked_ = false;
  time_graph_->NeedsUpdate();
}

void CaptureViewElement::OnDrag(int x, int y) {
  canvas_->ScreenToWorld(x, y, mouse_pos_cur_[0], mouse_pos_cur_[1]);
  time_graph_->NeedsUpdate();
}

}  // namespace orbit_gl