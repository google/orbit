// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_H_

#include "Batcher.h"
#include "PickingManager.h"
#include "TimeGraphLayout.h"

class TimeGraph;
class GlCanvas;

namespace orbit_gl {

/* Base class for UI elements drawn underneath the capture window. */
class CaptureViewElement : public Pickable {
 public:
  explicit CaptureViewElement(TimeGraph* time_graph, TimeGraphLayout* layout);
  virtual void Draw(GlCanvas* canvas, PickingMode /*picking_mode*/, float /*z_offset*/ = 0) {
    canvas_ = canvas;
  }

  virtual void UpdatePrimitives(Batcher* /*batcher*/, uint64_t /*min_tick*/, uint64_t /*max_tick*/,
                                PickingMode /*picking_mode*/, float /*z_offset*/ = 0){};

  void SetTimeGraph(TimeGraph* timegraph) { time_graph_ = timegraph; }
  [[nodiscard]] TimeGraph* GetTimeGraph() { return time_graph_; }

  [[nodiscard]] GlCanvas* GetCanvas() const { return canvas_; }

  virtual void SetPos(float x, float y) { pos_ = Vec2(x, y); }
  [[nodiscard]] Vec2 GetPos() const { return pos_; }
  void SetSize(float width, float height) { size_ = Vec2(width, height); }
  [[nodiscard]] Vec2 GetSize() const { return size_; }

  // Pickable
  void OnPick(int x, int y) override;
  void OnRelease() override;
  void OnDrag(int x, int y) override;
  [[nodiscard]] bool Draggable() override { return true; }

 protected:
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
