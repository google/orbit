// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_

#include "AccessibleInterfaceProvider.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "PickingManager.h"
#include "PrimitiveAssembler.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "TimelineInfoInterface.h"
#include "Viewport.h"

namespace orbit_gl {

struct ModifierKeys {
  bool ctrl = false;
  bool shift = false;
  bool alt = false;
};

/* Base class for UI elements drawn underneath the capture window. */
class CaptureViewElement : public Pickable, public AccessibleInterfaceProvider {
 public:
  explicit CaptureViewElement(CaptureViewElement* parent, const Viewport* viewport,
                              const TimeGraphLayout* layout);

  void UpdateLayout();

  [[nodiscard]] const orbit_gl::Viewport* GetViewport() const { return viewport_; }

  void SetPos(float x, float y);

  // TODO(b/185854980): This should not be virtual as soon as we have meaningful track children.
  [[nodiscard]] virtual Vec2 GetPos() const { return pos_; }

  void SetWidth(float width);
  [[nodiscard]] float GetWidth() const { return width_; }

  // Height should be defined in every particular capture view element.
  [[nodiscard]] virtual float GetHeight() const = 0;
  [[nodiscard]] Vec2 GetSize() const { return Vec2(GetWidth(), GetHeight()); }
  [[nodiscard]] virtual bool ShouldBeRendered() const { return GetVisible(); }

  void SetVisible(bool value);
  [[nodiscard]] bool GetVisible() const { return visible_; }

  // Pickable
  void OnPick(int x, int y) override;
  void OnRelease() override;
  void OnDrag(int x, int y) override;
  [[nodiscard]] bool Draggable() override { return true; }

  [[nodiscard]] bool ContainsPoint(const Vec2& pos);

  // This also checks IsMouseOver() for the parent, and only returns true if the mouse
  // position is included in all parents up to the root
  [[nodiscard]] bool IsMouseOver(const Vec2& mouse_pos);

  enum class EventResult { kHandled, kIgnored };
  [[nodiscard]] EventResult HandleMouseWheelEvent(const Vec2& mouse_pos, int delta,
                                                  const ModifierKeys& modifiers = ModifierKeys());

  [[nodiscard]] virtual CaptureViewElement* GetParent() const { return parent_; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetAllChildren() const { return {}; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetNonHiddenChildren() const;
  [[nodiscard]] std::vector<CaptureViewElement*> GetChildrenVisibleInViewport() const;

  virtual void RequestUpdate();

  enum LayoutFlags : uint32_t { kNone = 0, kScaleHorizontallyWithParent = 1 << 0 };

  [[nodiscard]] virtual uint32_t GetLayoutFlags() const { return kScaleHorizontallyWithParent; }
  [[nodiscard]] virtual float DetermineZOffset() const { return 0.f; }

 protected:
  struct DrawContext {
    uint64_t current_mouse_time_ns = 0;
    PickingMode picking_mode = PickingMode::kNone;
  };

  const orbit_gl::Viewport* viewport_;
  const TimeGraphLayout* layout_;

  Vec2 mouse_pos_last_click_;
  Vec2 mouse_pos_cur_;
  Vec2 picking_offset_ = Vec2(0, 0);
  bool picked_ = false;
  bool visible_ = true;

  void Draw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
            const DrawContext& draw_context);
  void UpdatePrimitives(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                        uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode);

  virtual void DoDraw(PrimitiveAssembler& /*primitive_assembler*/, TextRenderer& /*text_renderer*/,
                      const DrawContext& /*draw_context*/) {}

  virtual void DoUpdatePrimitives(PrimitiveAssembler& /*primitive_assembler*/,
                                  TextRenderer& /*text_renderer*/, uint64_t /*min_tick*/,
                                  uint64_t /*max_tick*/, PickingMode /*picking_mode*/) {}

  virtual void DoUpdateLayout() {}

  [[nodiscard]] virtual EventResult OnMouseWheel(const Vec2& mouse_pos, int delta,
                                                 const ModifierKeys& modifiers);

 private:
  float width_ = 0.;
  Vec2 pos_ = Vec2(0, 0);
  CaptureViewElement* parent_;
};
}  // namespace orbit_gl

#endif
