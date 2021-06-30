// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlSlider.h"

#include <GteVector.h>
#include <math.h>

#include <algorithm>

#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "PickingManager.h"

namespace orbit_gl {

const float GlSlider::kGradientFactor = 0.25f;

Vec2 GlSlider::GetPos() const {
  if (is_vertical_) {
    return Vec2(viewport_.GetScreenWidth() - pixel_height_, 0);
  } else {
    return Vec2(0, viewport_.GetScreenHeight() - pixel_height_);
  }
}

Vec2 GlSlider::GetSize() const {
  if (is_vertical_) {
    return Vec2(pixel_height_, viewport_.GetScreenHeight() - orthogonal_slider_size_);
  } else {
    return Vec2(viewport_.GetScreenWidth() - orthogonal_slider_size_, pixel_height_);
  }
}

void GlSlider::OnMouseEnter() { is_mouse_over_ = true; }

void GlSlider::OnMouseLeave() { is_mouse_over_ = false; }

bool GlSlider::ContainsScreenSpacePoint(int x, int y) const {
  return x >= GetPos()[0] && x <= GetPos()[0] + GetSize()[0] && y >= GetPos()[1] &&
         y <= GetPos()[1] + GetSize()[1];
}

GlSlider::GlSlider(Viewport& viewport, bool is_vertical)
    : is_vertical_(is_vertical),
      viewport_(viewport),
      pos_ratio_(0),
      right_edge_ratio_(0),
      length_ratio_(0),
      picking_pixel_offset_(0),
      selected_color_(75, 75, 75, 255),
      slider_color_(68, 68, 68, 255),
      bar_color_(61, 61, 61, 255),
      min_slider_pixel_length_(20),
      pixel_height_(20),
      orthogonal_slider_size_(20),
      slider_resize_pixel_margin_(6) {}

void GlSlider::SetNormalizedPosition(float start_ratio)  // [0,1]
{
  pos_ratio_ = std::clamp(start_ratio, 0.f, 1.f);
  right_edge_ratio_ = pos_ratio_ * (1.0f - length_ratio_) + length_ratio_;
}

void GlSlider::SetNormalizedLength(float length_ratio)  // [0,1]
{
  float min_length = static_cast<float>(min_slider_pixel_length_) / GetBarPixelLength();
  length_ratio_ = std::clamp(length_ratio, min_length, 1.f);
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
  float value = is_vertical_ ? y : x;
  float slider_pos = PosToPixel(pos_ratio_);
  float slider_right_pos = LenToPixel(right_edge_ratio_);

  switch (drag_type_) {
    case DragType::kNone:
      return;
    case DragType::kPan:
      SetNormalizedPosition(PixelToPos(value - picking_pixel_offset_));
      break;
    case DragType::kScaleMin: {
      float new_pos = std::clamp(value - picking_pixel_offset_, 0.f,
                                 slider_right_pos - min_slider_pixel_length_);
      SetNormalizedLength(PixelToLen(slider_right_pos - new_pos));
      SetNormalizedPosition(PixelToPos(new_pos));
      break;
    }
    case DragType::kScaleMax: {
      float len = GetBarPixelLength();
      SetNormalizedLength(
          PixelToLen(std::clamp(value + picking_pixel_offset_, 0.f, len) - slider_pos));
      SetNormalizedPosition(PixelToPos(slider_pos));
      break;
    }
  }

  if (drag_type_ != DragType::kPan && resize_callback_ != nullptr) {
    const float pos_as_len = PixelToLen(PosToPixel(pos_ratio_));
    resize_callback_(pos_as_len, pos_as_len + length_ratio_);
  }

  if (drag_callback_) {
    drag_callback_(pos_ratio_);
  }
}

void GlSlider::OnPick(int x, int y) {
  float value = is_vertical_ ? y : x;

  float slider_pos = PosToPixel(pos_ratio_);
  float slider_length = LenToPixel(length_ratio_);

  if (HandlePageScroll(value)) {
    drag_type_ = DragType::kNone;
    return;
  }

  if (value <= slider_pos + slider_resize_pixel_margin_ && can_resize_) {
    drag_type_ = DragType::kScaleMin;
    picking_pixel_offset_ = value - slider_pos;
  } else if (value >= slider_pos + slider_length - slider_resize_pixel_margin_ && can_resize_) {
    drag_type_ = DragType::kScaleMax;
    picking_pixel_offset_ = slider_pos + slider_length - value;
  } else {
    drag_type_ = DragType::kPan;
    picking_pixel_offset_ = value - slider_pos;
  }
}

void GlSlider::DrawBackground(Batcher& batcher, float x, float y, float width, float height) {
  const Color dark_border_color = GetDarkerColor(bar_color_);
  const float kEpsilon = 0.0001f;

  Box border_box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSliderBg - kEpsilon);
  batcher.AddBox(border_box, dark_border_color, shared_from_this());

  Box bar_box(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f), GlCanvas::kZValueSliderBg);
  batcher.AddBox(bar_box, bar_color_, shared_from_this());
}

void GlSlider::DrawSlider(Batcher& batcher, float x, float y, float width, float height,
                          ShadingDirection shading_direction, bool is_picked) {
  Color color = is_picked && drag_type_ == DragType::kPan ? selected_color_ : slider_color_;
  const Color dark_border_color = GetDarkerColor(bar_color_);
  const Color light_border_color = GetLighterColor(color);
  const float kEpsilon = 0.0001f;

  Box dark_border_box(Vec2(x, y), Vec2(width, height), GlCanvas::kZValueSlider - 2 * kEpsilon);

  batcher.AddBox(dark_border_box, dark_border_color, shared_from_this());

  Box light_border_box(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f),
                       GlCanvas::kZValueSlider - kEpsilon);

  batcher.AddBox(light_border_box, light_border_color, shared_from_this());

  // Slider itself
  batcher.AddShadedBox(Vec2(x + 2.f, y + 2.f), Vec2(width - 4.f, height - 4.f),
                       GlCanvas::kZValueSlider, color, shared_from_this(), shading_direction);
}

bool GlSlider::HandlePageScroll(float click_value) {
  float slider_pos = PosToPixel(pos_ratio_);
  float slider_length = LenToPixel(length_ratio_);

  if (click_value >= slider_pos && click_value <= slider_pos + slider_length) {
    return false;
  }

  if (click_value < slider_pos) {
    SetNormalizedPosition(PixelToPos(slider_pos - slider_length));
    if (drag_callback_ != nullptr) {
      drag_callback_(pos_ratio_);
    }
  }

  if (click_value > slider_pos + slider_length) {
    SetNormalizedPosition(PixelToPos(slider_pos + slider_length));
    if (drag_callback_ != nullptr) {
      drag_callback_(pos_ratio_);
    }
  }

  return true;
}

void GlVerticalSlider::Draw(Batcher& batcher, bool is_picked) {
  float x = viewport_.GetScreenWidth() - GetPixelHeight();

  float bar_pixel_len = GetBarPixelLength();
  float slider_height = ceilf(length_ratio_ * bar_pixel_len);
  float non_slider_height = bar_pixel_len - slider_height;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  // Background
  DrawBackground(batcher, x, GetOrthogonalSliderSize(), GetPixelHeight(), bar_pixel_len);

  float start = ceilf((1.0f - pos_ratio_) * non_slider_height + GetOrthogonalSliderSize());

  DrawSlider(batcher, x, start, GetPixelHeight(), slider_height, ShadingDirection::kRightToLeft,
             is_picked);
}

int GlVerticalSlider::GetBarPixelLength() const {
  return viewport_.GetScreenHeight() - GetOrthogonalSliderSize();
}

void GlHorizontalSlider::Draw(Batcher& batcher, bool is_picked) {
  static float y = 0;

  float bar_pixel_len = GetBarPixelLength();
  float slider_width = ceilf(length_ratio_ * bar_pixel_len);
  float non_slider_width = bar_pixel_len - slider_width;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  Color color = is_picked ? selected_color_ : slider_color_;
  const Color light_border_color = GetLighterColor(color);

  DrawBackground(batcher, 0, y, bar_pixel_len, GetPixelHeight());

  float start = floorf(pos_ratio_ * non_slider_width);

  DrawSlider(batcher, start, y, slider_width, GetPixelHeight(), ShadingDirection::kTopToBottom,
             is_picked);

  const float kEpsilon = 0.0001f;

  // Left / right resize arrows and separator
  const float kHeightFactor = 2.f;
  // Triangle is 3 pixels smaller than the area - there is a 2 pixel border around the scrollbar,
  // and there is no margin between the border and the triangle to make it as big as possible.
  // On the other side, there is a 1 pixel margin between the triangle and the vertical line
  float tri_size = slider_resize_pixel_margin_ - 3.f;
  tri_size = std::min(tri_size, pixel_height_ - tri_size * kHeightFactor - 2.f);
  const float tri_y_offset = (pixel_height_ - tri_size * kHeightFactor) / 2.f;
  const Color kWhite = GetLighterColor(GetLighterColor(bar_color_));
  const float z = GlCanvas::kZValueSlider + 2 * kEpsilon;
  const float x = start;
  const float width = slider_width;

  batcher.AddTriangle(
      Triangle(Vec3(x + width - tri_size - 2.f, kHeightFactor * tri_size + tri_y_offset, z),
               Vec3(x + width - 2.f, tri_y_offset + kHeightFactor / 2.f * tri_size, z),
               Vec3(x + width - tri_size - 2.f, tri_y_offset, z)),
      kWhite, shared_from_this());
  batcher.AddVerticalLine(Vec2(x + width - slider_resize_pixel_margin_, 2.f), pixel_height_ - 4.f,
                          z, kWhite, shared_from_this());

  batcher.AddTriangle(Triangle(Vec3(x + tri_size + 2.f, kHeightFactor * tri_size + tri_y_offset, z),
                               Vec3(x + tri_size + 2.f, tri_y_offset, z),
                               Vec3(x + 2.f, tri_y_offset + kHeightFactor / 2.f * tri_size, z)),
                      kWhite, shared_from_this());
  batcher.AddVerticalLine(Vec2(x + slider_resize_pixel_margin_ + 1, 2.f), pixel_height_ - 4.f, z,
                          kWhite, shared_from_this());

  // Highlight the scale part of the slider
  if (is_picked) {
    if (drag_type_ == DragType::kScaleMax) {
      batcher.AddShadedBox(Vec2(x + width - slider_resize_pixel_margin_, 2),
                           Vec2(slider_resize_pixel_margin_ - 2, pixel_height_ - 4),
                           GlCanvas::kZValueSlider + kEpsilon, selected_color_,
                           ShadingDirection::kTopToBottom);
    } else if (drag_type_ == DragType::kScaleMin) {
      batcher.AddShadedBox(Vec2(x + 2, 2), Vec2(slider_resize_pixel_margin_ - 2, pixel_height_ - 4),
                           GlCanvas::kZValueSlider + kEpsilon, selected_color_,
                           ShadingDirection::kTopToBottom);
    }
  }
}

int GlHorizontalSlider::GetBarPixelLength() const {
  return viewport_.GetScreenWidth() - GetOrthogonalSliderSize();
}

}  // namespace orbit_gl