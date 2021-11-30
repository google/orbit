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
  explicit TriangleToggle(StateChangeHandler handler, orbit_gl::Viewport* viewport,
                          TimeGraphLayout* layout, Track* track);
  ~TriangleToggle() override = default;

  TriangleToggle() = delete;
  TriangleToggle(const TriangleToggle&) = delete;
  TriangleToggle& operator=(const TriangleToggle&) = delete;
  TriangleToggle(TriangleToggle&&) = delete;
  TriangleToggle& operator=(TriangleToggle&&) = delete;

  [[nodiscard]] float GetHeight() const override { return height_; }
  void SetHeight(float height) { height_ = height; }

  // Pickable
  void OnRelease() override;

  void SetCollapsed(bool is_collapsed) { is_collapsed_ = is_collapsed; }
  [[nodiscard]] bool IsCollapsed() const { return is_collapsed_; }
  void SetIsCollapsible(bool is_collapsible) { is_collapsible_ = is_collapsible; }
  [[nodiscard]] bool IsCollapsible() const { return is_collapsible_; }

  [[nodiscard]] uint32_t GetLayoutFlags() const override { return LayoutFlags::kNone; }

 protected:
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  float height_ = 20;
  bool is_collapsed_ = false;
  bool is_collapsible_ = true;

  StateChangeHandler handler_;
};

#endif  // ORBIT_GL_TRIANGLE_TOGGLE_H_
