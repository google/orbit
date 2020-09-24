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

  bool Draggable() override { return true; }
  void SetSliderRatio(float a_Start);       // [0,1]
  void SetSliderWidthRatio(float a_Ratio);  // [0,1]
  void SetCanvas(GlCanvas* a_Canvas) { m_Canvas = a_Canvas; }
  Color GetBarColor() const { return m_SliderColor; }
  void SetPixelHeight(float height) { m_PixelHeight = height; }
  float GetPixelHeight() const { return m_PixelHeight; }

  void SetOrthogonalSliderSize(float size) { orthogonal_slider_size_ = size; }
  float GetOrthogonalSliderSize() { return orthogonal_slider_size_; }

  typedef std::function<void(float)> DragCallback;
  void SetDragCallback(DragCallback a_Callback) { m_DragCallback = a_Callback; }

 protected:
  static Color GetLighterColor(const Color& color);
  static Color GetDarkerColor(const Color& color);

  void DrawBackground(GlCanvas* canvas, float x, float y, float width, float height);
  void DrawSlider(GlCanvas* canvas, float x, float y, float width, float height, bool picking,
                  ShadingDirection shading_direction);

 protected:
  TextBox m_Slider;
  TextBox m_Bar;
  GlCanvas* m_Canvas;
  float m_Ratio;
  float m_Length;
  float m_PickingRatio;
  DragCallback m_DragCallback;
  Color m_SelectedColor;
  Color m_SliderColor;
  Color m_BarColor;
  float m_MinSliderPixelWidth;
  float m_PixelHeight;
  float orthogonal_slider_size_;
};

class GlVerticalSlider : public GlSlider {
 public:
  GlVerticalSlider() : GlSlider(){};
  ~GlVerticalSlider(){};

  void OnPick(int x, int y) override;
  void OnDrag(int x, int y) override;
  void Draw(GlCanvas* a_Canvas, PickingMode picking_mode) override;
};

class GlHorizontalSlider : public GlSlider {
 public:
  GlHorizontalSlider() : GlSlider(){};
  ~GlHorizontalSlider(){};

  void OnPick(int x, int y) override;
  void OnDrag(int x, int y) override;
  void Draw(GlCanvas* a_Canvas, PickingMode picking_mode) override;
};