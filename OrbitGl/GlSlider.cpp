// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlSlider.h"

#include <algorithm>

#include "GlCanvas.h"
#include "PickingManager.h"

GlSlider::GlSlider()
    : canvas_(nullptr),
      ratio_(0),
      length_(0),
      picking_ratio_(0),
      selected_color_(75, 75, 75, 255),
      slider_color_(68, 68, 68, 255),
      bar_color_(61, 61, 61, 255),
      min_slider_pixel_width_(20),
      pixel_height_(20),
      orthogonal_slider_size_(20) {}

void GlSlider::SetSliderRatio(float ratio)  // [0,1]
{
  ratio_ = ratio;
}

void GlSlider::SetSliderWidthRatio(float width_radio)  // [0,1]
{
  float minWidth = min_slider_pixel_width_ / static_cast<float>(canvas_->getWidth());
  length_ = std::max(width_radio, minWidth);
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
  const Color dark_border_color = GetDarkerColor(bar_color_);

  // Dark border
  {
    Box box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSliderBg - 0.0001f);
    batcher->AddBox(box, dark_border_color, shared_from_this());
  }
  // Background itself
  {
    Box box(Vec2(x + 1, y + 1), Vec2(width - 2, height - 2), GlCanvas::kZValueSliderBg);
    batcher->AddBox(box, bar_color_, shared_from_this());
  }
}

void GlSlider::DrawSlider(GlCanvas* canvas, float x, float y, float width, float height,
                          ShadingDirection shading_direction) {
  Batcher* batcher = canvas->GetBatcher();
  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? selected_color_ : slider_color_;
  const Color dark_border_color = GetDarkerColor(bar_color_);
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
  float canvasHeight = canvas_->getHeight();
  float sliderHeight = length_ * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderY = ratio_ * nonSliderHeight;
  picking_ratio_ = (static_cast<float>(y) - sliderY) / sliderHeight;
}

void GlVerticalSlider::OnDrag(int /*x*/, int y) {
  float canvasHeight = canvas_->getHeight();
  float sliderHeight = length_ * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderTopHeight = picking_ratio_ * length_ * canvasHeight;
  float newY = static_cast<float>(y) - sliderTopHeight;
  float ratio = newY / nonSliderHeight;

  ratio_ = clamp(ratio, 0.f, 1.f);

  if (drag_callback_) {
    drag_callback_(ratio_);
  }
}

void GlVerticalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  CHECK(canvas == canvas_);

  const bool picking = picking_mode != PickingMode::kNone;
  float x = canvas_->getWidth() - GetPixelHeight();

  float canvasHeight = canvas_->getHeight() - GetOrthogonalSliderSize();
  float sliderHeight = length_ * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  // Background
  if (!picking) {
    DrawBackground(canvas, x, GetOrthogonalSliderSize(), GetPixelHeight(), canvasHeight);
  }

  float start = (1.0f - ratio_) * nonSliderHeight + GetOrthogonalSliderSize();

  DrawSlider(canvas, x, start, GetPixelHeight(), sliderHeight, ShadingDirection::kRightToLeft);
}

void GlHorizontalSlider::OnPick(int x, int /*y*/) {
  float canvasWidth = canvas_->getWidth();
  float sliderWidth = length_ * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderX = ratio_ * nonSliderWidth;
  picking_ratio_ = (static_cast<float>(x) - sliderX) / sliderWidth;
}

void GlHorizontalSlider::OnDrag(int x, int /*y*/) {
  float canvasWidth = canvas_->getWidth();
  float sliderWidth = length_ * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderLeftWidth = picking_ratio_ * length_ * canvasWidth;
  float newX = static_cast<float>(x) - sliderLeftWidth;
  float ratio = newX / nonSliderWidth;

  ratio_ = clamp(ratio, 0.f, 1.f);

  if (drag_callback_) {
    drag_callback_(ratio_);
  }
}

void GlHorizontalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;

  canvas_ = canvas;
  static float y = 0;

  float canvasWidth = canvas_->getWidth() - GetOrthogonalSliderSize();
  float sliderWidth = length_ * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? selected_color_ : slider_color_;
  const Color light_border_color = GetLighterColor(color);

  if (!picking) {
    DrawBackground(canvas, 0, y, canvasWidth, GetPixelHeight());
  }

  float start = ratio_ * nonSliderWidth;

  DrawSlider(canvas, start, y, sliderWidth, GetPixelHeight(), ShadingDirection::kTopToBottom);
}