// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_

#include "AccessibleInterfaceProvider.h"
#include "Batcher.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "PickingManager.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

class TimeGraph;
class GlCanvas;

namespace orbit_gl {

/* Base class for UI elements drawn underneath the capture window. */
class CaptureViewElement : public Pickable, public AccessibleInterfaceProvider {
 public:
  explicit CaptureViewElement(CaptureViewElement* parent, TimeGraph* time_graph,
                              orbit_gl::Viewport* viewport, TimeGraphLayout* layout);
  virtual void Draw(GlCanvas* canvas, PickingMode /*picking_mode*/, float /*z_offset*/ = 0) {
    canvas_ = canvas;
  }

  virtual void UpdatePrimitives(Batcher* /*batcher*/, uint64_t /*min_tick*/, uint64_t /*max_tick*/,
                                PickingMode /*picking_mode*/, float /*z_offset*/ = 0){};

  [[nodiscard]] TimeGraph* GetTimeGraph() { return time_graph_; }

  [[nodiscard]] orbit_gl::Viewport* GetViewport() const { return viewport_; }
  [[nodiscard]] GlCanvas* GetCanvas() const { return canvas_; }
  void SetCanvas(GlCanvas* canvas) { canvas_ = canvas; }

  void SetPos(float x, float y) { pos_ = Vec2(x, y); }
  // TODO(b/185854980): This should not be virtual as soon as we have meaningful track children.
  [[nodiscard]] virtual Vec2 GetPos() const { return pos_; }
  void SetSize(float width, float height) { size_ = Vec2(width, height); }
  // TODO(b/185854980): This should not be virtual as soon as we have meaningful track children.
  [[nodiscard]] virtual Vec2 GetSize() const { return size_; }

  // Pickable
  void OnPick(int x, int y) override;
  void OnRelease() override;
  void OnDrag(int x, int y) override;
  [[nodiscard]] bool Draggable() override { return true; }

  [[nodiscard]] CaptureViewElement* GetParent() const { return parent_; }

 protected:
  CaptureViewElement* parent_;
  orbit_gl::Viewport* viewport_;
  TimeGraphLayout* layout_;

  GlCanvas* canvas_ = nullptr;
  TimeGraph* time_graph_;
  Vec2 pos_ = Vec2(0, 0);
  Vec2 size_ = Vec2(0, 0);

  Vec2 mouse_pos_last_click_;
  Vec2 mouse_pos_cur_;
  Vec2 picking_offset_ = Vec2(0, 0);
  bool picked_ = false;
};
}  // namespace orbit_gl

#endif
