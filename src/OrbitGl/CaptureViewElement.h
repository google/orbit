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
  // TODO(b/230442062): This method shouldn't be virtual. Fix it after refactoring sliders.
  [[nodiscard]] virtual float GetWidth() const { return width_; }

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

  // This also checks ShouldReactToMouseOver() for the parent, and only returns true if the mouse
  // position is included in all parents up to the root
  [[nodiscard]] bool ShouldReactToMouseOver(const Vec2& mouse_pos) const;

  [[nodiscard]] bool IsMouseOver() const { return is_mouse_over_; }

  enum class EventType {
    kInvalidEvent,
    kMouseMove,
    kMouseLeave,
    kMouseWheelUp,
    kMouseWheelDown,
    kLeftUp,
    kLeftDown,
    kRightUp,
    kRightDown
  };

  const inline static Vec2 kOutsidePosition{std::numeric_limits<float>::max(),
                                            std::numeric_limits<float>::max()};
  struct MouseEvent {
    EventType event_type = EventType::kInvalidEvent;
    Vec2 mouse_position = kOutsidePosition;
    bool left = false;
    bool right = false;
    bool middle = false;
  };

  enum class EventResult { kHandled, kIgnored };
  [[nodiscard]] EventResult HandleMouseEvent(MouseEvent mouse_event,
                                             const ModifierKeys& modifiers = ModifierKeys());

  [[nodiscard]] virtual CaptureViewElement* GetParent() const { return parent_; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetAllChildren() const { return {}; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetNonHiddenChildren() const;
  [[nodiscard]] std::vector<CaptureViewElement*> GetChildrenVisibleInViewport() const;

  // Specifies the type of render data that has been invalidated and needs update
  enum class RequestUpdateScope {
    kDraw = 1,  // Only "::Draw" will be called next frame. Use this when neither timers nor
                // time-dependent data needs to be updated
    kDrawAndUpdatePrimitives =
        2  // Both ::Draw and ::UpdatePrimitives will be called next frame. Use this whenever timers
           // or time-dependent data needs to be updated
  };

  // Indicate that data has changed that requires an update of the UI.
  // This will bubble up and notify the parent. In the next frame, *all* elements will be redrawn
  // (i.e. will have `Draw` and / or `UpdatePrimitives` called, depending on the
  // `RequestUpdateScope`), not only the ones that called this method.
  //
  // Usage:
  // * Call this whenever your element performs any action that requires redrawing.
  // * Make sure to ONLY call it when actual changes are present - e.g. for setters, make sure that
  //   the newly set value differs from the previous one before calling this to avoid unneeded
  //   redraws.
  void RequestUpdate(RequestUpdateScope scope = RequestUpdateScope::kDrawAndUpdatePrimitives);

  enum LayoutFlags : uint32_t { kNone = 0, kScaleHorizontallyWithParent = 1 << 0 };

  [[nodiscard]] virtual uint32_t GetLayoutFlags() const { return kScaleHorizontallyWithParent; }
  [[nodiscard]] virtual float DetermineZOffset() const { return 0.f; }

  [[nodiscard]] bool HasLayoutChanged() const { return has_layout_changed_; }

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

  bool draw_requested_ = false;
  bool update_primitives_requested_ = false;
  bool has_layout_changed_ = false;

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

  [[nodiscard]] bool ContainsPoint(const Vec2& pos) const;
  [[nodiscard]] virtual EventResult OnMouseWheel(const Vec2& mouse_pos, int delta,
                                                 const ModifierKeys& modifiers);
  [[nodiscard]] virtual EventResult OnMouseMove(const Vec2& mouse_pos);
  [[nodiscard]] virtual EventResult OnMouseEnter();
  [[nodiscard]] virtual EventResult OnMouseLeave();

 private:
  bool is_mouse_over_ = false;

  float width_ = 0.;
  Vec2 pos_ = Vec2(0, 0);
  CaptureViewElement* parent_;

  friend class CaptureViewElementTester;
};
}  // namespace orbit_gl

#endif
