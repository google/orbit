// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_

#include "AccessibleInterfaceProvider.h"
#include "Batcher.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "PickingManager.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

class TimeGraph;

namespace orbit_gl {

/* Base class for UI elements drawn underneath the capture window. */
class CaptureViewElement : public Pickable, public AccessibleInterfaceProvider {
 public:
  explicit CaptureViewElement(CaptureViewElement* parent, TimeGraph* time_graph,
                              orbit_gl::Viewport* viewport, TimeGraphLayout* layout);

  struct DrawContext {
    constexpr const static uint32_t kMaxIndentationLevel = 5;
    uint64_t current_mouse_time_ns;
    PickingMode picking_mode;
    uint32_t indentation_level;
    float z_offset = 0;

    [[nodiscard]] DrawContext IncreasedIndentationLevel() const {
      auto copy = *this;
      copy.indentation_level = std::min(kMaxIndentationLevel, copy.indentation_level + 1);
      return copy;
    };

    [[nodiscard]] DrawContext UpdatedZOffset(float z_offset) const {
      auto copy = *this;
      copy.z_offset = z_offset;
      return copy;
    }
  };

  void Draw(Batcher& batcher, TextRenderer& text_renderer, const DrawContext& draw_context);

  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0);
  void UpdateLayout();

  [[nodiscard]] TimeGraph* GetTimeGraph() { return time_graph_; }

  [[nodiscard]] orbit_gl::Viewport* GetViewport() const { return viewport_; }

  // TODO(b/185854980): This should not be virtual as soon as we have meaningful track children.
  virtual void SetPos(float x, float y) { pos_ = Vec2(x, y); }
  // TODO(b/185854980): This should not be virtual as soon as we have meaningful track children.
  [[nodiscard]] virtual Vec2 GetPos() const { return pos_; }

  // This will set the width of all child elements to 100% by default
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

  [[nodiscard]] virtual CaptureViewElement* GetParent() const { return parent_; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetChildren() const { return {}; }
  virtual void RequestUpdate();

 protected:
  orbit_gl::Viewport* viewport_;
  TimeGraphLayout* layout_;

  TimeGraph* time_graph_;

  Vec2 mouse_pos_last_click_;
  Vec2 mouse_pos_cur_;
  Vec2 picking_offset_ = Vec2(0, 0);
  bool picked_ = false;
  bool visible_ = true;

  virtual void DoDraw(Batcher& /*batcher*/, TextRenderer& /*text_renderer*/,
                      const DrawContext& /*draw_context*/) {}

  virtual void DoUpdatePrimitives(Batcher* /*batcher*/, uint64_t /*min_tick*/,
                                  uint64_t /*max_tick*/, PickingMode /*picking_mode*/,
                                  float /*z_offset*/ = 0) {}

  virtual void DoUpdateLayout() {}

 private:
  float width_ = 0.;
  Vec2 pos_ = Vec2(0, 0);
  CaptureViewElement* parent_;
};
}  // namespace orbit_gl

#endif
