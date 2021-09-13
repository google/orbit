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

void CaptureViewElement::Draw(Batcher& batcher, TextRenderer& text_renderer,
                              const DrawContext& draw_context) {
  DoDraw(batcher, text_renderer, draw_context);

  const DrawContext inner_draw_context = draw_context.IncreasedIndentationLevel();
  for (auto& child : GetChildren()) {
    if (child->ShouldBeRendered()) {
      child->Draw(batcher, text_renderer, inner_draw_context);
    }
  }
}

void CaptureViewElement::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                          PickingMode picking_mode, float z_offset) {
  DoUpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  for (auto& child : GetChildren()) {
    if (child->ShouldBeRendered()) {
      child->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
    }
  }
}

void CaptureViewElement::UpdateLayout() {
  for (auto& child : GetChildren()) {
    child->UpdateLayout();
  }
  DoUpdateLayout();
}

void CaptureViewElement::SetWidth(float width) {
  if (width != width_) {
    width_ = width;

    for (auto& child : GetChildren()) {
      child->SetWidth(width);
    }
    RequestUpdate();
  }
}

void CaptureViewElement::SetVisible(bool value) {
  if (visible_ == value) return;

  visible_ = value;
  RequestUpdate();
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