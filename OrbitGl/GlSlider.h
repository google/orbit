// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>

#include "Batcher.h"
#include "Params.h"
#include "PickingManager.h"
#include "TextBox.h"

class GlCanvas;

class GlSlider : public Pickable, public std::enable_shared_from_this<GlSlider> {
 public:
  GlSlider();
  ~GlSlider(){};

  [[nodiscard]] bool Draggable() override { return true; }

  void SetCanvas(GlCanvas* canvas) { canvas_ = canvas; }
  void SetSliderPosRatio(float start_ratio);  // [0,1]

  void SetSliderLengthRatio(float length_ratio);  // [0,1]
  [[nodiscard]] Color GetBarColor() const { return slider_color_; }
  void SetPixelHeight(float height) { pixel_height_ = height; }
  [[nodiscard]] float GetPixelHeight() const { return pixel_height_; }

  void SetOrthogonalSliderSize(float size) { orthogonal_slider_size_ = size; }
  [[nodiscard]] float GetOrthogonalSliderSize() { return orthogonal_slider_size_; }

  typedef std::function<void(float)> DragCallback;
  void SetDragCallback(DragCallback callback) { drag_callback_ = callback; }

  typedef std::function<void(float, float)> ResizeCallback;
  void SetResizeCallback(ResizeCallback callback) { resize_callback_ = callback; }

  [[nodiscard]] float GetPosRatio() { return pos_ratio_; }
  [[nodiscard]] float GetLengthRatio() { return length_ratio_; }

  void OnPick(int x, int y) override;
  void OnDrag(int x, int y) override;

  [[nodiscard]] float GetMinSliderPixelLength() { return min_slider_pixel_length_; }

  float GetPixelPos() { return PosToPixel(pos_ratio_); }
  float GetPixelLength() { return LenToPixel(length_ratio_); }

 protected:
  static Color GetLighterColor(const Color& color);
  static Color GetDarkerColor(const Color& color);

  void DrawBackground(GlCanvas* canvas, float x, float y, float width, float height);
  void DrawSlider(GlCanvas* canvas, float x, float y, float width, float height,
                  ShadingDirection shading_direction);
  virtual int GetRelevantMouseDim(int x, int y) = 0;

  [[nodiscard]] virtual float GetCanvasEdgeLength() = 0;

  [[nodiscard]] float PixelToLen(float value) { return value / GetCanvasEdgeLength(); }
  [[nodiscard]] float LenToPixel(float value) { return value * GetCanvasEdgeLength(); }
  [[nodiscard]] float PixelToPos(float value) { return value / LenToPixel(1.0f - length_ratio_); }
  [[nodiscard]] float PosToPixel(float value) { return value * LenToPixel(1.0f - length_ratio_); }

  [[nodiscard]] bool HandlePageScroll(int click_value);

 protected:
  static const float kGradientFactor;

  GlCanvas* canvas_;

  float pos_ratio_;
  float right_edge_ratio_;
  float length_ratio_;
  float picking_pixel_offset_;

  DragCallback drag_callback_;
  ResizeCallback resize_callback_;

  Color selected_color_;
  Color slider_color_;
  Color bar_color_;
  int min_slider_pixel_length_;
  float pixel_height_;
  float orthogonal_slider_size_;

  int slider_resize_pixel_margin_;

  enum class DragType { kPan, kScaleMin, kScaleMax, kNone };
  DragType drag_type_;
};

class GlVerticalSlider : public GlSlider {
 public:
  GlVerticalSlider() : GlSlider(){};
  ~GlVerticalSlider(){};

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;

 protected:
  [[nodiscard]] float GetCanvasEdgeLength() override;
  [[nodiscard]] int GetRelevantMouseDim(int x, int y) override { return y; }
};

class GlHorizontalSlider : public GlSlider {
 public:
  GlHorizontalSlider() : GlSlider(){};
  ~GlHorizontalSlider(){};

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;

 protected:
  [[nodiscard]] float GetCanvasEdgeLength() override;
  [[nodiscard]] int GetRelevantMouseDim(int x, int y) override { return x; }
};