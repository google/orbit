// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlSlider.h"

#include <algorithm>

#include "GlCanvas.h"
#include "PickingManager.h"

GlSlider::GlSlider()
    : m_Canvas(nullptr),
      m_Ratio(0),
      m_Length(0),
      m_PickingRatio(0),
      m_SelectedColor(40, 40, 40, 255),
      m_SliderColor(50, 50, 50, 255),
      m_BarColor(60, 60, 60, 255),
      m_MinSliderPixelWidth(20),
      m_PixelHeight(20),
      m_Vertical(false)

{}

void GlSlider::SetSliderRatio(float a_Ratio)  // [0,1]
{
  m_Ratio = a_Ratio;
}

void GlSlider::SetSliderWidthRatio(float a_WidthRatio)  // [0,1]
{
  float minWidth =
      m_MinSliderPixelWidth / static_cast<float>(m_Canvas->getWidth());
  m_Length = std::max(a_WidthRatio, minWidth);
}

void GlSlider::OnPick(int a_X, int a_Y) {
  m_Vertical ? OnPickVertical(a_X, a_Y) : OnPickHorizontal(a_X, a_Y);
}

void GlSlider::OnPickHorizontal(int a_X, int /*a_Y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float x = static_cast<float>(a_X);
  float sliderX = m_Ratio * nonSliderWidth;
  m_PickingRatio = (x - sliderX) / sliderWidth;
}

void GlSlider::OnPickVertical(int /*a_X*/, int a_Y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float y = a_Y;
  float sliderY = m_Ratio * nonSliderHeight;
  m_PickingRatio = (y - sliderY) / sliderHeight;
}

void GlSlider::OnDrag(int a_X, int a_Y) {
  m_Vertical ? OnDragVertical(a_X, a_Y) : OnDragHorizontal(a_X, a_Y);
}

void GlSlider::OnDragVertical(int /*a_X*/, int a_Y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderTopHeight = m_PickingRatio * m_Length * canvasHeight;
  float newY = static_cast<float>(a_Y) - sliderTopHeight;
  float ratio = newY / nonSliderHeight;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

void GlSlider::OnDragHorizontal(int a_X, int /*a_Y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderLeftWidth = m_PickingRatio * m_Length * canvasWidth;
  float newX = static_cast<float>(a_X) - sliderLeftWidth;
  float ratio = newX / nonSliderWidth;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

void GlSlider::Draw(GlCanvas* a_Canvas, PickingMode picking_mode) {
  m_Vertical ? DrawVertical(a_Canvas, picking_mode)
             : DrawHorizontal(a_Canvas, picking_mode);
}

void GlSlider::DrawHorizontal(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;

  m_Canvas = canvas;
  Batcher* batcher = canvas->GetBatcher();

  static float y = 0;

  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  // Bar
  if (!picking) {
    Box box(Vec2(0, y), Vec2(canvasWidth, GetPixelHeight()), 0.f);
    batcher->AddBox(box, m_BarColor, shared_from_this());
  }

  float start = m_Ratio * nonSliderWidth;
  float stop = start + sliderWidth;

  Color color = canvas->GetPickingManager().IsThisElementPicked(this)
                    ? m_SelectedColor
                    : m_SliderColor;

  Box box(Vec2(start, y), Vec2(stop - start, GetPixelHeight()), 0.f);
  batcher->AddBox(box, color, shared_from_this());
}

void GlSlider::DrawVertical(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;
  m_Canvas = canvas;
  Batcher* batcher = canvas->GetBatcher();

  float x = m_Canvas->getWidth() - GetPixelHeight();

  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  // Bar
  if (!picking) {
    Box box(Vec2(x, 0), Vec2(GetPixelHeight(), canvasHeight), 0.f);
    batcher->AddBox(box, m_BarColor, shared_from_this());
  }

  float start = canvasHeight - m_Ratio * nonSliderHeight;
  float stop = start - sliderHeight;

  Color color = canvas->GetPickingManager().IsThisElementPicked(this)
                    ? m_SelectedColor
                    : m_SliderColor;

  Box box(Vec2(x, start), Vec2(GetPixelHeight(), stop - start), 0.f);
  batcher->AddBox(box, color, shared_from_this());
}
