// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRIANGLE_TOGGLE_H_
#define ORBIT_GL_TRIANGLE_TOGGLE_H_

#include <functional>
#include <memory>

#include "CaptureViewElement.h"
#include "CoreMath.h"
#include "Viewport.h"

class Track;

class TriangleToggle : public orbit_gl::CaptureViewElement,
                       public std::enable_shared_from_this<TriangleToggle> {
 public:
  using StateChangeHandler = std::function<void(bool)>;
  explicit TriangleToggle(StateChangeHandler handler, TimeGraph* time_graph,
                          orbit_gl::Viewport* viewport, TimeGraphLayout* layout, Track* track,
                          float size);
  ~TriangleToggle() override = default;

  TriangleToggle() = delete;
  TriangleToggle(const TriangleToggle&) = delete;
  TriangleToggle& operator=(const TriangleToggle&) = delete;
  TriangleToggle(TriangleToggle&&) = delete;
  TriangleToggle& operator=(TriangleToggle&&) = delete;

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;

  // Pickable
  void OnRelease() override;

  void SetCollapsed(bool is_collapsed) { is_collapsed_ = is_collapsed; }
  [[nodiscard]] bool IsCollapsed() const { return is_collapsed_; }
  void SetIsCollapsible(bool is_collapsible) { is_collapsible_ = is_collapsible; }
  [[nodiscard]] bool IsCollapsible() const { return is_collapsible_; }

 protected:
  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  // Ideally this should be `CaptureViewElement* parent`, but as the track is not the parent, but
  // the virtual `TrackTab`, which is a child of the track itself, we stick to track here.
  // We require explicit knowledge about the parent.
  Track* track_;

  bool is_collapsed_ = false;
  bool is_collapsible_ = true;

  StateChangeHandler handler_;
};

#endif  // ORBIT_GL_TRIANGLE_TOGGLE_H_
