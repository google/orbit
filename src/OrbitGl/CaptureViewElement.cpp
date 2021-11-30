// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElement.h"

#include "Introspection/Introspection.h"
#include "Viewport.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(CaptureViewElement* parent, Viewport* viewport,
                                       TimeGraphLayout* layout)
    : viewport_(viewport), layout_(layout), parent_(parent) {
  CHECK(layout != nullptr);
}

void CaptureViewElement::Draw(Batcher& batcher, TextRenderer& text_renderer,
                              const DrawContext& draw_context) {
  ORBIT_SCOPE_FUNCTION;

  batcher.PushTranslation(0, 0, DetermineZOffset());
  text_renderer.PushTranslation(0, 0, DetermineZOffset());

  DoDraw(batcher, text_renderer, draw_context);

  for (CaptureViewElement* child : GetChildrenVisibleInViewport()) {
    child->Draw(batcher, text_renderer, draw_context);
  }

  text_renderer.PopTranslation();
  batcher.PopTranslation();
}

void CaptureViewElement::UpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                          uint64_t min_tick, uint64_t max_tick,
                                          PickingMode picking_mode) {
  ORBIT_SCOPE_FUNCTION;

  batcher.PushTranslation(0, 0, DetermineZOffset());
  text_renderer.PushTranslation(0, 0, DetermineZOffset());

  DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);

  for (CaptureViewElement* child : GetChildrenVisibleInViewport()) {
    if (child->ShouldBeRendered()) {
      child->UpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);
    }
  }

  text_renderer.PopTranslation();
  batcher.PopTranslation();
}

void CaptureViewElement::UpdateLayout() {
  for (CaptureViewElement* child : GetAllChildren()) {
    child->UpdateLayout();
  }
  DoUpdateLayout();
}

void CaptureViewElement::SetPos(float x, float y) {
  const Vec2 pos = Vec2(x, y);
  if (pos == pos_) return;

  pos_ = pos;
  RequestUpdate();
}

void CaptureViewElement::SetWidth(float width) {
  if (width != width_) {
    width_ = width;

    for (auto& child : GetAllChildren()) {
      if (child->GetLayoutFlags() & LayoutFlags::kScaleHorizontallyWithParent) {
        child->SetWidth(width);
      }
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
  mouse_pos_last_click_ = viewport_->ScreenToWorld(Vec2i(x, y));
  picking_offset_ = mouse_pos_last_click_ - pos_;
  mouse_pos_cur_ = mouse_pos_last_click_;
  picked_ = true;
}

void CaptureViewElement::OnRelease() {
  picked_ = false;
  RequestUpdate();
}

void CaptureViewElement::OnDrag(int x, int y) {
  mouse_pos_cur_ = viewport_->ScreenToWorld(Vec2i(x, y));
  RequestUpdate();
}

std::vector<CaptureViewElement*> CaptureViewElement::GetNonHiddenChildren() const {
  std::vector<CaptureViewElement*> result;
  for (CaptureViewElement* child : GetAllChildren()) {
    if (child->ShouldBeRendered()) {
      result.push_back(child);
    }
  }

  return result;
}

std::vector<CaptureViewElement*> CaptureViewElement::GetChildrenVisibleInViewport() const {
  std::vector<CaptureViewElement*> result;
  for (CaptureViewElement* child : GetNonHiddenChildren()) {
    float child_top_y = child->GetPos()[1];
    float child_bottom_y = child_top_y + child->GetHeight();
    float screen_top_y = 0;
    float screen_bottom_y = screen_top_y + viewport_->GetWorldHeight();
    if (child_top_y < screen_bottom_y && child_bottom_y > screen_top_y) {
      result.push_back(child);
    }
  }
  return result;
}

void CaptureViewElement::RequestUpdate() {
  if (parent_ != nullptr) {
    parent_->RequestUpdate();
  }
}

}  // namespace orbit_gl