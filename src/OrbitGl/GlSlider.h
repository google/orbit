// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_SLIDER_H_
#define ORBIT_GL_GL_SLIDER_H_

#include <functional>
#include <memory>
#include <utility>

#include "CaptureViewElement.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "PrimitiveAssembler.h"
#include "Viewport.h"

namespace orbit_gl {

class GlSlider : public CaptureViewElement, public std::enable_shared_from_this<GlSlider> {
 public:
  ~GlSlider() override = default;

  [[nodiscard]] bool Draggable() override { return true; }

  void SetNormalizedPosition(float start_ratio);  // [0,1]
  void SetNormalizedLength(float length_ratio);   // [0,1]

  [[nodiscard]] Color GetBarColor() const { return slider_color_; }

  void SetPixelHeight(int height) { pixel_height_ = height; }
  [[nodiscard]] int GetPixelHeight() const { return pixel_height_; }

  void SetOrthogonalSliderPixelHeight(int size) { orthogonal_slider_size_ = size; }
  [[nodiscard]] int GetOrthogonalSliderSize() const { return orthogonal_slider_size_; }

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

  [[nodiscard]] float GetMinSliderPixelLength() const { return min_slider_pixel_length_; }

  [[nodiscard]] float GetSliderPixelPos() const { return PosToPixel(pos_ratio_); }
  [[nodiscard]] float GetSliderPixelLength() const { return LenToPixel(length_ratio_); }

  [[nodiscard]] bool CanResize() const { return can_resize_; }

  void OnMouseEnter();
  void OnMouseLeave();
  void OnMouseMove(int x, int y);
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

  [[nodiscard]] bool PosIsInMinResizeArea(int x, int y) const;
  [[nodiscard]] bool PosIsInMaxResizeArea(int x, int y) const;
  [[nodiscard]] bool PosIsInSlider(int x, int y) const;

  [[nodiscard]] float PixelToLen(float value) const { return value / GetBarPixelLength(); }
  [[nodiscard]] float LenToPixel(float value) const { return value * GetBarPixelLength(); }
  [[nodiscard]] float PixelToPos(float value) const {
    return length_ratio_ < 1.0f ? value / LenToPixel(1.0f - length_ratio_) : 0.f;
  }
  [[nodiscard]] float PosToPixel(float value) const {
    return value * LenToPixel(1.0f - length_ratio_);
  }

  [[nodiscard]] bool HandlePageScroll(float click_value);

 protected:
  [[nodiscard]] virtual float GetBarPixelLength() const = 0;

  static const float kGradientFactor;
  const bool is_vertical_;
  bool is_mouse_over_ = false;

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
  int min_slider_pixel_length_;
  int pixel_height_;
  int orthogonal_slider_size_;

  bool can_resize_ = false;

  int slider_resize_pixel_margin_;

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

  [[nodiscard]] float GetWidth() const override { return pixel_height_; }

  [[nodiscard]] float GetHeight() const override {
    return viewport_->GetScreenHeight() - GetPos()[1] - orthogonal_slider_size_;
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

  [[nodiscard]] float GetWidth() const override {
    return viewport_->GetScreenWidth() - orthogonal_slider_size_;
  }
  [[nodiscard]] float GetHeight() const override { return pixel_height_; }

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 protected:
  [[nodiscard]] float GetBarPixelLength() const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_GL_SLIDER_H_
