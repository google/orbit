// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/CaptureViewElement.h"

#include <GteVector.h>
#include <absl/strings/str_cat.h>

#include <algorithm>
#include <atomic>
#include <string_view>
#include <tuple>
#include <utility>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

CaptureViewElement::CaptureViewElement(CaptureViewElement* parent, const Viewport* viewport,
                                       const TimeGraphLayout* layout)
    : viewport_(viewport), layout_(layout), parent_(parent) {
  ORBIT_CHECK(layout != nullptr);

  static std::atomic<uint32_t> uid_counter = 0;
  uid_ = uid_counter++;
}

void CaptureViewElement::Draw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                              const DrawContext& draw_context) {
  ORBIT_SCOPE_FUNCTION;

  draw_requested_ = false;
  RenderGroups previous_groups = PreRender(primitive_assembler, text_renderer);

  DoDraw(primitive_assembler, text_renderer, draw_context);

  for (CaptureViewElement* child : GetChildrenVisibleInViewport()) {
    child->Draw(primitive_assembler, text_renderer, draw_context);
  }

  PostRender(std::move(previous_groups), primitive_assembler, text_renderer);
}

void CaptureViewElement::UpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                          TextRenderer& text_renderer, uint64_t min_tick,
                                          uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_FUNCTION;

  update_primitives_requested_ = false;
  RenderGroups previous_groups = PreRender(primitive_assembler, text_renderer);

  DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick, picking_mode);

  for (CaptureViewElement* child : GetChildrenVisibleInViewport()) {
    if (child->ShouldBeRendered()) {
      child->UpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick, picking_mode);
    }
  }

  PostRender(std::move(previous_groups), primitive_assembler, text_renderer);
}

CaptureViewElement::EventResult CaptureViewElement::OnMouseWheel(
    const Vec2& /*mouse_pos*/, int /*delta*/, const ModifierKeys& /*modifiers*/) {
  return EventResult::kIgnored;
}

CaptureViewElement::EventResult CaptureViewElement::OnMouseMove(const Vec2& mouse_pos) {
  if (ContainsPointRecursively(mouse_pos) && !is_mouse_over_) {
    std::ignore = OnMouseEnter();
  }

  // Ignore this event and let it be handled by its ancestors as well.
  return EventResult::kIgnored;
}

CaptureViewElement::EventResult CaptureViewElement::OnMouseEnter() {
  is_mouse_over_ = true;
  return EventResult::kIgnored;
}

CaptureViewElement::EventResult CaptureViewElement::OnMouseLeave() {
  is_mouse_over_ = false;
  mouse_pos_cur_ = kInvalidPosition;
  return EventResult::kIgnored;
}

void CaptureViewElement::UpdateLayout() {
  has_layout_changed_ = false;

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
    if ((child->GetLayoutFlags() & LayoutFlags::kScaleHorizontallyWithParent) != 0u) {
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
  RequestUpdate(RequestUpdateScope::kDraw);
}

void CaptureViewElement::OnDrag(int x, int y) {
  mouse_pos_cur_ = viewport_->ScreenToWorld(Vec2i(x, y));
  RequestUpdate(RequestUpdateScope::kDraw);
}

bool CaptureViewElement::ContainsPoint(const Vec2& pos) const {
  return pos[0] >= GetPos()[0] && pos[0] < GetPos()[0] + GetSize()[0] && pos[1] >= GetPos()[1] &&
         pos[1] < GetPos()[1] + GetSize()[1];
}

bool CaptureViewElement::ContainsPointRecursively(const Vec2& point) const {
  if (parent_ != nullptr && !parent_->ContainsPointRecursively(point)) {
    return false;
  }

  return ContainsPoint(point);
}

CaptureViewElement::EventResult CaptureViewElement::HandleMouseEvent(
    const MouseEvent& mouse_event, const ModifierKeys& modifiers) {
  Vec2 mouse_pos = mouse_event.mouse_position;

  if (mouse_event.event_type != MouseEventType::kMouseLeave &&
      !ContainsPointRecursively(mouse_pos)) {
    return EventResult::kIgnored;
  }

  for (CaptureViewElement* child : GetAllChildren()) {
    if (child->IsMouseOver() && !child->ContainsPointRecursively(mouse_pos)) {
      std::ignore = child->HandleMouseEvent(MouseEvent{MouseEventType::kMouseLeave});
    }
    if (child->ContainsPointRecursively(mouse_pos) &&
        child->HandleMouseEvent(mouse_event, modifiers) == EventResult::kHandled) {
      return EventResult::kHandled;
    }
  }

  mouse_pos_cur_ = mouse_pos;
  switch (mouse_event.event_type) {
    case MouseEventType::kMouseMove: {
      if (!mouse_event.left && !mouse_event.right && !mouse_event.middle) {
        return OnMouseMove(mouse_pos);
      }
      // Currently being handled using PickingManager (OnPick and OnDrag).
      return EventResult::kIgnored;
    }
    case MouseEventType::kMouseLeave:
      return OnMouseLeave();
    case MouseEventType::kMouseWheelUp:
      return OnMouseWheel(mouse_pos, /* delta= */ 1, modifiers);
    case MouseEventType::kMouseWheelDown:
      return OnMouseWheel(mouse_pos, /* delta= */ -1, modifiers);
    case MouseEventType::kLeftUp:
    case MouseEventType::kLeftDown:
    case MouseEventType::kRightUp:
    case MouseEventType::kRightDown:
      // Currently being handled using PickingManager (OnPick, OnDrag, OnRelease).
      return EventResult::kIgnored;
    case MouseEventType::kInvalidEvent:
    default:
      ORBIT_ERROR("Invalid Mouse Event");
      break;
  }
  return EventResult::kIgnored;
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

void CaptureViewElement::RequestUpdate(RequestUpdateScope scope) {
  switch (scope) {
    case orbit_gl::CaptureViewElement::RequestUpdateScope::kDraw:
      draw_requested_ = true;
      break;
    case orbit_gl::CaptureViewElement::RequestUpdateScope::kDrawAndUpdatePrimitives:
      draw_requested_ = true;
      update_primitives_requested_ = true;
      break;
    default:
      ORBIT_UNREACHABLE();
  }
  has_layout_changed_ = true;

  if (parent_ != nullptr) {
    parent_->RequestUpdate(scope);
  }
}

CaptureViewElement::RenderGroups CaptureViewElement::PreRender(
    PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer) {
  primitive_assembler.PushTranslation(0, 0, DetermineZOffset());
  text_renderer.PushTranslation(0, 0, DetermineZOffset());

  BatchRenderGroupStateManager* manager = primitive_assembler.GetRenderGroupManager();

  RenderGroups result{primitive_assembler.GetCurrentRenderGroupName(),
                      text_renderer.GetCurrentRenderGroupName()};

  if (RequestSeparateRenderGroup()) {
    constexpr std::string_view kGroupPrefix = "|rg_cve_";

    const std::string new_batcher_render_group_name =
        absl::StrCat(result.batcher_render_group_name, kGroupPrefix, std::to_string(GetUid()));
    primitive_assembler.SetCurrentRenderGroupName(new_batcher_render_group_name);

    BatchRenderGroupState parent_state = manager->GetGroupState(result.batcher_render_group_name);
    BatchRenderGroupState state = parent_state;
    state.stencil.pos = {GetPos()[0], GetPos()[1]};
    state.stencil.size = {GetSize()[0], GetSize()[1]};
    state.stencil.enabled = true;
    state.stencil = ClipStencil(state.stencil, parent_state.stencil);
    manager->SetGroupState(new_batcher_render_group_name, state);

    const std::string new_text_render_group_name =
        absl::StrCat(result.text_render_group_name, kGroupPrefix, std::to_string(GetUid()));
    text_renderer.SetCurrentRenderGroupName(new_text_render_group_name);

    if (new_text_render_group_name != new_batcher_render_group_name) {
      parent_state = manager->GetGroupState(result.text_render_group_name);
      state = parent_state;
      state.stencil.pos = {GetPos()[0], GetPos()[1]};
      state.stencil.size = {GetSize()[0], GetSize()[1]};
      state.stencil.enabled = true;
      state.stencil = ClipStencil(state.stencil, parent_state.stencil);
      manager->SetGroupState(new_text_render_group_name, state);
    }
  }

  return result;
}

void CaptureViewElement::PostRender(RenderGroups&& previous_groups,
                                    PrimitiveAssembler& primitive_assembler,
                                    TextRenderer& text_renderer) {
  if (RequestSeparateRenderGroup()) {
    primitive_assembler.SetCurrentRenderGroupName(previous_groups.batcher_render_group_name);
    text_renderer.SetCurrentRenderGroupName(previous_groups.text_render_group_name);
  }
  text_renderer.PopTranslation();
  primitive_assembler.PopTranslation();
}

}  // namespace orbit_gl