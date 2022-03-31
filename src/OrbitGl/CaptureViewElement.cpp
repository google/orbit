// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElement.h"

#include "Introspection/Introspection.h"
#include "Viewport.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(CaptureViewElement* parent, const Viewport* viewport,
                                       const TimeGraphLayout* layout)
    : viewport_(viewport), layout_(layout), parent_(parent) {
  ORBIT_CHECK(layout != nullptr);
}

void CaptureViewElement::Draw(PrimitiveAssembler& batcher, TextRenderer& text_renderer,
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

void CaptureViewElement::UpdatePrimitives(PrimitiveAssembler& batcher, TextRenderer& text_renderer,
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

CaptureViewElement::EventResult CaptureViewElement::OnMouseWheel(
    const Vec2& /*mouse_pos*/, int /*delta*/, const ModifierKeys& /*modifiers*/) {
  return EventResult::kIgnored;
}

void CaptureViewElement::UpdateLayout() {
  // Perform any layout changes of this element
  DoUpdateLayout();

  // Recurse into children
  for (CaptureViewElement* child : GetAllChildren()) {
    child->UpdateLayout();
  }
}

void CaptureViewElement::SetPos(float x, float y) {
  const Vec2 pos = Vec2(x, y);
  if (pos == pos_) return;

  pos_ = pos;
  RequestUpdate();
}

void CaptureViewElement::SetWidth(float width) {
  if (width == width_) return;

  for (auto& child : GetAllChildren()) {
    if (child->GetLayoutFlags() & LayoutFlags::kScaleHorizontallyWithParent) {
      child->SetWidth(width);
    }
  }

  width_ = width;
  RequestUpdate();
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

bool CaptureViewElement::ContainsPoint(const Vec2& pos) {
  return pos[0] >= GetPos()[0] && pos[0] <= GetPos()[0] + GetSize()[0] && pos[1] >= GetPos()[1] &&
         pos[1] <= GetPos()[1] + GetSize()[1];
}

bool CaptureViewElement::IsMouseOver(const Vec2& mouse_pos) {
  if (parent_ != nullptr && !parent_->IsMouseOver(mouse_pos)) {
    return false;
  }

  return ContainsPoint(mouse_pos);
}

CaptureViewElement::EventResult CaptureViewElement::HandleMouseWheelEvent(
    const Vec2& mouse_pos, int delta, const ModifierKeys& modifiers) {
  if (!IsMouseOver(mouse_pos)) {
    return EventResult::kIgnored;
  }

  for (CaptureViewElement* child : GetAllChildren()) {
    if (child->HandleMouseWheelEvent(mouse_pos, delta, modifiers) == EventResult::kHandled) {
      return EventResult::kHandled;
    }
  }

  return OnMouseWheel(mouse_pos, delta, modifiers);
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