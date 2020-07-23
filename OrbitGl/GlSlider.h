// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>

#include "Params.h"
#include "PickingManager.h"
#include "TextBox.h"

class GlCanvas;

class GlSlider : public Pickable {
 public:
  GlSlider();
  ~GlSlider(){};

  void OnPick(int a_X, int a_Y) override;
  void OnDrag(int a_X, int a_Y) override;
  void Draw(GlCanvas* a_Canvas, PickingMode picking_mode) override;
  bool Draggable() override { return true; }
  void SetSliderRatio(float a_Start);       // [0,1]
  void SetSliderWidthRatio(float a_Ratio);  // [0,1]
  void SetCanvas(GlCanvas* a_Canvas) { m_Canvas = a_Canvas; }
  Color GetBarColor() const { return m_SliderColor; }
  void SetPixelHeight(float height) { m_PixelHeight = height; }
  float GetPixelHeight() const { return m_PixelHeight; }
  void SetVertical() { m_Vertical = true; }

  typedef std::function<void(float)> DragCallback;
  void SetDragCallback(DragCallback a_Callback) { m_DragCallback = a_Callback; }

 protected:
  void DrawHorizontal(GlCanvas* a_Canvas, PickingMode picking_mode);
  void DrawVertical(GlCanvas* a_Canvas, PickingMode picking_mode);
  void OnDragHorizontal(int a_X, int a_Y);
  void OnDragVertical(int a_X, int a_Y);
  void OnPickHorizontal(int a_X, int a_Y);
  void OnPickVertical(int a_X, int a_Y);

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
  bool m_Vertical;
};
