// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_SLIDER_H_
#define ORBIT_GL_GL_SLIDER_H_

#include <GteVector.h>

#include <functional>
#include <memory>
#include <utility>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class GlSlider : public CaptureViewElement, public std::enable_shared_from_this<GlSlider> {
 public:
  ~GlSlider() override = default;

  [[nodiscard]] bool Draggable() override { return true; }

  void SetNormalizedPosition(float start_ratio);  // [0,1]
  void SetNormalizedLength(float length_ratio);   // [0,1]
  [[nodiscard]] float GetSliderWidth() const { return layout_->GetSliderWidth(); }

  // Parameter: Position in [0, 1], relative to the size of the current data window
  using DragCallback = std::function<void(float)>;
  void SetDragCallback(DragCallback callback) { drag_callback_ = std::move(callback); }

  // Parameters: Start and End of the slider in [0, 1], relative to the full data window
  using ResizeCallback = std::function<void(float, float)>;
  void SetResizeCallback(ResizeCallback callback) { resize_callback_ = std::move(callback); }

  [[nodiscard]] float GetPosRatio() const { return pos_ratio_; }
  [[nodiscard]] float GetLengthRatio() const { return length_ratio_; }

  void OnPick(int x, int y) override;
  void OnDrag(int x, int y) override;

  [[nodiscard]] float GetMinSliderPixelLength() const { return layout_->GetMinSliderLength(); }

  [[nodiscard]] float GetSliderPixelPos() const { return PosToPixel(pos_ratio_); }
  [[nodiscard]] float GetSliderPixelLength() const { return LenToPixel(length_ratio_); }

  [[nodiscard]] bool CanResize() const { return can_resize_; }

  [[nodiscard]] bool ContainsScreenSpacePoint(int x, int y) const;

 protected:
  explicit GlSlider(CaptureViewElement* parent, const Viewport* viewport,
                    const TimeGraphLayout* layout, TimelineInfoInterface* timeline_info,
                    bool is_vertical);

  static Color GetLighterColor(const Color& color);
  static Color GetDarkerColor(const Color& color);

  void DrawBackground(PrimitiveAssembler& primitive_assembler, float x, float y, float width,
                      float height);
  void DrawSlider(PrimitiveAssembler& primitive_assembler, float x, float y, float width,
                  float height, ShadingDirection shading_direction);

  [[nodiscard]] EventResult OnMouseMove(const Vec2& mouse_pos) override;
  [[nodiscard]] EventResult OnMouseEnter() override;
  [[nodiscard]] EventResult OnMouseLeave() override;
  [[nodiscard]] bool HandlePageScroll(float click_value);

  [[nodiscard]] float GetSliderResizeMargin() const { return layout_->GetSliderResizeMargin(); }
  [[nodiscard]] virtual float GetBarPixelLength() const = 0;
  [[nodiscard]] bool PosIsInMinResizeArea(const Vec2& pos) const;
  [[nodiscard]] bool PosIsInMaxResizeArea(const Vec2& pos) const;
  [[nodiscard]] bool PosIsInSlider(const Vec2& pos) const;

  [[nodiscard]] float PixelToLen(float value) const { return value / GetBarPixelLength(); }
  [[nodiscard]] float LenToPixel(float value) const { return value * GetBarPixelLength(); }
  [[nodiscard]] float PixelToPos(float value) const {
    return length_ratio_ < 1.0f ? value / LenToPixel(1.0f - length_ratio_) : 0.f;
  }
  [[nodiscard]] float PosToPixel(float value) const {
    return value * LenToPixel(1.0f - length_ratio_);
  }

  static constexpr float kGradientFactor = 0.25f;
  const bool is_vertical_;

  TimelineInfoInterface* timeline_info_;

  float pos_ratio_;  // Position of the data window in [0, 1], relative to the visible data size
  float right_edge_ratio_;  // Right edge of the data in [0, 1], relative to the visible data size
  float length_ratio_;      // Length of the slider, relative to the max data size
  float picking_pixel_offset_;  // Offset of the mouse cursor from the left of the slider in pixels

  DragCallback drag_callback_;
  ResizeCallback resize_callback_;

  Color selected_color_;
  Color slider_color_;
  Color bar_color_;

  bool can_resize_ = false;

  enum class DragType { kPan, kScaleMin, kScaleMax, kNone };
  DragType drag_type_ = DragType::kNone;
};

class GlVerticalSlider : public GlSlider {
 public:
  GlVerticalSlider(CaptureViewElement* parent, const Viewport* viewport,
                   const TimeGraphLayout* layout, TimelineInfoInterface* timeline_info)
      : GlSlider(parent, viewport, layout, timeline_info, true) {}

  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  [[nodiscard]] float GetWidth() const override { return GetSliderWidth(); }

  [[nodiscard]] float GetHeight() const override {
    return viewport_->GetScreenHeight() - GetPos()[1] - layout_->GetSliderWidth();
  }

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 protected:
  [[nodiscard]] float GetBarPixelLength() const override;
};

class GlHorizontalSlider : public GlSlider {
 public:
  GlHorizontalSlider(CaptureViewElement* parent, const Viewport* viewport,
                     const TimeGraphLayout* layout, TimelineInfoInterface* timeline_info)
      : GlSlider(parent, viewport, layout, timeline_info, false) {
    can_resize_ = true;
  }

  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  [[nodiscard]] float GetHeight() const override { return GetSliderWidth(); }

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 protected:
  [[nodiscard]] float GetBarPixelLength() const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_GL_SLIDER_H_
