// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlSlider.h"

#include <algorithm>

#include "GlCanvas.h"
#include "PickingManager.h"

const float GlSlider::kGradientFactor = 0.25f;

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
  const float kLocalGradientFactor = 1.0f + kGradientFactor;
  return Color(static_cast<unsigned char>(color[0] * kLocalGradientFactor),
               static_cast<unsigned char>(color[1] * kLocalGradientFactor),
               static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255);
}

Color GlSlider::GetDarkerColor(const Color& color) {
  const float kLocalGradientFactor = 1.0f - kGradientFactor;
  return Color(static_cast<unsigned char>(color[0] * kLocalGradientFactor),
               static_cast<unsigned char>(color[1] * kLocalGradientFactor),
               static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255);
}

void GlSlider::DrawBackground(GlCanvas* canvas, float x, float y, float width, float height) {
  Batcher* batcher = canvas->GetBatcher();
  const Color dark_border_color = GetDarkerColor(bar_color_);
  const float kEpsilon = 0.0001f;

  Box border_box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSliderBg - kEpsilon);
  batcher->AddBox(border_box, dark_border_color, shared_from_this());

  Box bar_box(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f), GlCanvas::kZValueSliderBg);
  batcher->AddBox(bar_box, bar_color_, shared_from_this());
}

void GlSlider::DrawSlider(GlCanvas* canvas, float x, float y, float width, float height,
                          ShadingDirection shading_direction) {
  Batcher* batcher = canvas->GetBatcher();
  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? selected_color_ : slider_color_;
  const Color dark_border_color = GetDarkerColor(bar_color_);
  const Color light_border_color = GetLighterColor(color);
  const float kEpsilon = 0.0001f;

  Box dark_border_box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSlider - 2 * kEpsilon);

  batcher->AddBox(dark_border_box, dark_border_color, shared_from_this());

  Box light_border_box(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f),
                       GlCanvas::kZValueSlider - kEpsilon);

  batcher->AddBox(light_border_box, light_border_color, shared_from_this());

  // Slider itself
  batcher->AddShadedBox(Vec2(x + 2.f, y + 2.f), Vec2(width - 4.f, height - 4.f),
                        GlCanvas::kZValueSlider, color, shared_from_this(), shading_direction);
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