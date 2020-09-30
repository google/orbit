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
      pos_ratio_(0),
      length_ratio_(0),
      picking_pixel_offset_(0),
      selected_color_(75, 75, 75, 255),
      slider_color_(68, 68, 68, 255),
      bar_color_(61, 61, 61, 255),
      min_slider_pixel_length_(20),
      slider_resize_pixel_margin_(5),
      pixel_height_(20),
      orthogonal_slider_size_(20) {}

void GlSlider::SetSliderPosRatio(float start_ratio)  // [0,1]
{
  pos_ratio_ = clamp(start_ratio, 0.f, 1.f);
  right_edge_ratio_ = pos_ratio_ * (1.0f - length_ratio_) + length_ratio_;
}

void GlSlider::SetSliderLengthRatio(float length_ratio)  // [0,1]
{
  float min_length = static_cast<float>(min_slider_pixel_length_) / GetCanvasEdgeLength();
  length_ratio_ = clamp(length_ratio, min_length, 1.f - pos_ratio_ * (1.f - length_ratio_));
  right_edge_ratio_ = pos_ratio_ * (1.0f - length_ratio_) + length_ratio_;
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

void GlSlider::OnDrag(int x, int y) {
  float value = GetRelevantMouseDim(x, y);
  float slider_pos = PosToPixel(pos_ratio_);

  switch (drag_type_) {
    case DragType::kNone:
      return;
    case DragType::kPan:
      SetSliderPosRatio(PixelToPos(value - picking_pixel_offset_));
      break;
    case DragType::kScaleMin: {
      float new_pos = clamp(value - picking_pixel_offset_, 0.f, GetCanvasEdgeLength());
      SetSliderLengthRatio(PixelToLen(LenToPixel(right_edge_ratio_) - new_pos));
      SetSliderPosRatio(PixelToPos(new_pos));
      break;
    }
    case DragType::kScaleMax:
      SetSliderLengthRatio(PixelToLen(
          clamp(value + picking_pixel_offset_, 0.f, GetCanvasEdgeLength()) - slider_pos));
      SetSliderPosRatio(PixelToPos(slider_pos));
      break;
  }

  if (drag_type_ != DragType::kPan && resize_callback_ != nullptr) {
    resize_callback_(length_ratio_);
  }

  if (drag_callback_) {
    drag_callback_(pos_ratio_);
  }
}

void GlSlider::OnPick(int x, int y) {
  float value = GetRelevantMouseDim(x, y);

  float slider_pos = PosToPixel(pos_ratio_);
  float slider_length = LenToPixel(length_ratio_);

  if (HandlePageScroll(value)) {
    drag_type_ = DragType::kNone;
    return;
  }

  if (value <= slider_pos + slider_resize_pixel_margin_) {
    drag_type_ = DragType::kScaleMin;
    picking_pixel_offset_ = value - slider_pos;
  } else if (value >= slider_pos + slider_length - slider_resize_pixel_margin_) {
    drag_type_ = DragType::kScaleMax;
    picking_pixel_offset_ = slider_pos + slider_length - value;
  } else {
    drag_type_ = DragType::kPan;
    picking_pixel_offset_ = value - slider_pos;
  }
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

bool GlSlider::HandlePageScroll(int click_value) {
  float slider_pos = PosToPixel(pos_ratio_);
  float slider_length = LenToPixel(length_ratio_);

  if (click_value >= slider_pos && click_value <= slider_pos + slider_length) {
    return false;
  }

  if (click_value < slider_pos) {
    SetSliderPosRatio(PixelToPos(slider_pos - slider_length));
    if (drag_callback_ != nullptr) {
      drag_callback_(pos_ratio_);
    }
  }

  if (click_value > slider_pos + slider_length) {
    SetSliderPosRatio(PixelToPos(slider_pos + slider_length));
    if (drag_callback_ != nullptr) {
      drag_callback_(pos_ratio_);
    }
  }

  return true;
}

void GlVerticalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  CHECK(canvas == canvas_);

  const bool picking = picking_mode != PickingMode::kNone;
  float x = canvas_->GetWidth() - GetPixelHeight();

  float canvasHeight = canvas_->GetHeight() - GetOrthogonalSliderSize();
  float sliderHeight = length_ratio_ * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  // Background
  if (!picking) {
    DrawBackground(canvas, x, GetOrthogonalSliderSize(), GetPixelHeight(), canvasHeight);
  }

  float start = (1.0f - pos_ratio_) * nonSliderHeight + GetOrthogonalSliderSize();

  DrawSlider(canvas, x, start, GetPixelHeight(), sliderHeight, ShadingDirection::kRightToLeft);
}

float GlVerticalSlider::GetCanvasEdgeLength() {
  return canvas_->GetHeight() - GetOrthogonalSliderSize();
}

void GlHorizontalSlider::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;

  canvas_ = canvas;
  static float y = 0;

  float canvasWidth = canvas_->GetWidth() - GetOrthogonalSliderSize();
  float sliderWidth = length_ratio_ * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  Color color =
      canvas->GetPickingManager().IsThisElementPicked(this) ? selected_color_ : slider_color_;
  const Color light_border_color = GetLighterColor(color);

  if (!picking) {
    DrawBackground(canvas, 0, y, canvasWidth, GetPixelHeight());
  }

  float start = pos_ratio_ * nonSliderWidth;

  DrawSlider(canvas, start, y, sliderWidth, GetPixelHeight(), ShadingDirection::kTopToBottom);
}

float GlHorizontalSlider::GetCanvasEdgeLength() {
  return canvas_->GetWidth() - GetOrthogonalSliderSize();
}
