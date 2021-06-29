// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_SLIDER_H_
#define ORBIT_GL_GL_SLIDER_H_

#include <functional>
#include <memory>
#include <utility>

#include "Batcher.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "Viewport.h"

namespace orbit_gl {

class GlSlider : public Pickable, public std::enable_shared_from_this<GlSlider> {
 public:
  ~GlSlider() override = default;

  [[nodiscard]] bool Draggable() override { return true; }
  virtual void Draw(Batcher& batcher, bool is_picked) = 0;

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

  [[nodiscard]] Vec2 GetPos() const;
  [[nodiscard]] Vec2 GetSize() const;

  void OnMouseEnter();
  void OnMouseLeave();
  void OnMouseMove(int x, int y);
  [[nodiscard]] bool ContainsScreenSpacePoint(int x, int y) const;

 protected:
  explicit GlSlider(Viewport& viewport, bool is_vertical);

  static Color GetLighterColor(const Color& color);
  static Color GetDarkerColor(const Color& color);

  void DrawBackground(Batcher& batcher, float x, float y, float width, float height);
  void DrawSlider(Batcher& batcher, float x, float y, float width, float height,
                  ShadingDirection shading_direction, bool is_picked);

  [[nodiscard]] virtual int GetBarPixelLength() const = 0;

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
  static const float kGradientFactor;
  const bool is_vertical_;
  bool is_mouse_over_ = false;
  Vec2i mouse_pos_ = Vec2i(-1, -1);

  Viewport& viewport_;

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
  GlVerticalSlider(Viewport& viewport) : GlSlider(viewport, true) {}

  void Draw(Batcher& batcher, bool is_picked) override;

 protected:
  [[nodiscard]] int GetBarPixelLength() const override;
};

class GlHorizontalSlider : public GlSlider {
 public:
  GlHorizontalSlider(Viewport& viewport) : GlSlider(viewport, false) { can_resize_ = true; }

  void Draw(Batcher& batcher, bool is_picked) override;

 protected:
  [[nodiscard]] int GetBarPixelLength() const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_GL_SLIDER_H_
