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
      m_SelectedColor(75, 75, 75, 255),
      m_SliderColor(68, 68, 68, 255),
      m_BarColor(61, 61, 61, 255),
      m_MinSliderPixelWidth(20),
      m_PixelHeight(20),
      orthogonal_slider_size_(20) {}

void GlSlider::SetSliderRatio(float a_Ratio)  // [0,1]
{
  m_Ratio = a_Ratio;
}

void GlSlider::SetSliderWidthRatio(float a_WidthRatio)  // [0,1]
{
  float minWidth = m_MinSliderPixelWidth / static_cast<float>(m_Canvas->getWidth());
  m_Length = std::max(a_WidthRatio, minWidth);
}

Color GlSlider::GetLighterColor(const Color& color) {
  return Color(static_cast<unsigned char>(color[0] * 1.25),
               static_cast<unsigned char>(color[1] * 1.25),
               static_cast<unsigned char>(color[2] * 1.25), 255);
}

Color GlSlider::GetDarkerColor(const Color& color) {
  return Color(static_cast<unsigned char>(color[0] * 0.75),
               static_cast<unsigned char>(color[1] * 0.75),
               static_cast<unsigned char>(color[2] * 0.75), 255);
}

void GlSlider::DrawBackground(GlCanvas* canvas, float x, float y, float width, float height) {
  Batcher* batcher = canvas->GetBatcher();
  const Color dark_border_color = GetDarkerColor(m_BarColor);

  // Dark border
  {
    Box box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSliderBg - 0.0001f);
    batcher->AddBox(box, dark_border_color, shared_from_this());
  }
  // Background itself
  {
    Box box(Vec2(x + 1, y + 1), Vec2(width - 2, height - 2), GlCanvas::kZValueSliderBg);
    batcher->AddBox(box, m_BarColor, shared_from_this());
  }
}

void GlSlider::DrawSlider(GlCanvas* canvas, float x, float y, float width, float height,
                          bool picking, ShadingDirection shading_direction) {
  Batcher* batcher = canvas->GetBatcher();
  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? m_SelectedColor : m_SliderColor;
  const Color dark_border_color = GetDarkerColor(m_BarColor);
  const Color light_border_color = GetLighterColor(color);

  // Slider
  {
    // Dark border
    Box box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSlider - 0.0002f);

    batcher->AddBox(box, dark_border_color, shared_from_this());
  }
  {
    // Light inner border
    Box box(Vec2(x + 1, y + 1), Vec2(width - 2, height - 2), GlCanvas::kZValueSlider - 0.0001f);

    batcher->AddBox(box, light_border_color, shared_from_this());
  }
  // Slider itself
  batcher->AddShadedBox(Vec2(x + 2, y + 2), Vec2(width - 4, height - 4), GlCanvas::kZValueSlider,
                        color, shared_from_this(), shading_direction);
}

void GlVerticalSlider::OnPick(int /*x*/, int y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderY = m_Ratio * nonSliderHeight;
  m_PickingRatio = (static_cast<float>(y) - sliderY) / sliderHeight;
}

void GlVerticalSlider::OnDrag(int /*x*/, int y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderTopHeight = m_PickingRatio * m_Length * canvasHeight;
  float newY = static_cast<float>(y) - sliderTopHeight;
  float ratio = newY / nonSliderHeight;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

void GlVerticalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;
  m_Canvas = canvas;
  Batcher* batcher = canvas->GetBatcher();

  float x = m_Canvas->getWidth() - GetPixelHeight();

  float canvasHeight = m_Canvas->getHeight() - GetOrthogonalSliderSize();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  const Color dark_border_color = GetDarkerColor(m_BarColor);

  // Background
  if (!picking) {
    DrawBackground(canvas, x, GetOrthogonalSliderSize(), GetPixelHeight(), canvasHeight);
  }

  float start = (1.0f - m_Ratio) * nonSliderHeight + GetOrthogonalSliderSize();

  DrawSlider(canvas, x, start, GetPixelHeight(), sliderHeight, picking,
             ShadingDirection::kRightToLeft);
}

void GlHorizontalSlider::OnPick(int x, int /*y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderX = m_Ratio * nonSliderWidth;
  m_PickingRatio = (static_cast<float>(x) - sliderX) / sliderWidth;
}

void GlHorizontalSlider::OnDrag(int x, int /*y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderLeftWidth = m_PickingRatio * m_Length * canvasWidth;
  float newX = static_cast<float>(x) - sliderLeftWidth;
  float ratio = newX / nonSliderWidth;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

void GlHorizontalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;

  m_Canvas = canvas;
  Batcher* batcher = canvas->GetBatcher();

  static float y = 0;

  float canvasWidth = m_Canvas->getWidth() - GetOrthogonalSliderSize();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  const Color dark_border_color = GetDarkerColor(m_BarColor);

  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? m_SelectedColor : m_SliderColor;
  const Color light_border_color = GetLighterColor(color);

  if (!picking) {
    DrawBackground(canvas, 0, y, canvasWidth, GetPixelHeight());
  }

  float start = m_Ratio * nonSliderWidth;

  DrawSlider(canvas, start, y, sliderWidth, GetPixelHeight(), picking,
             ShadingDirection::kTopToBottom);
}