// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_VERTICAL_SIZER_H_
#define ORBIT_GL_VERTICAL_SIZER_H_

#include <stdint.h>

#include <functional>
#include <memory>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

// The VerticalSizer is a vertical line that we can move horizontally to trigger an action which is
// user-defined in the `on_drag_callback`.
class VerticalSizer : public orbit_gl::CaptureViewElement,
                      public std::enable_shared_from_this<VerticalSizer> {
 public:
  explicit VerticalSizer(CaptureViewElement* parent, const orbit_gl::Viewport* viewport,
                         const TimeGraphLayout* layout,
                         std::function<void(int /*x*/, int /*y*/)> on_drag_callback);
  ~VerticalSizer() override = default;

  // Pickable
  void OnDrag(int /*x*/, int /*y*/) override;

  [[nodiscard]] float GetHeight() const override { return height_; }
  void SetHeight(float height);

  [[nodiscard]] uint32_t GetLayoutFlags() const override { return LayoutFlags::kNone; }

 protected:
  void DoDraw(orbit_gl::PrimitiveAssembler& primitive_assembler,
              orbit_gl::TextRenderer& text_renderer, const DrawContext& draw_context) override;
  [[nodiscard]] EventResult OnMouseEnter() override;
  [[nodiscard]] EventResult OnMouseLeave() override;
  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  float height_ = 0.f;
  std::function<void(int /*x*/, int /*y*/)> on_drag_callback_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_VERTICAL_SIZER_H_
