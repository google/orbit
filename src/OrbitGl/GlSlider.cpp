// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GlSlider.h"

#include <GteVector.h>

#include <QCursor>
#include <QGuiApplication>
#include <Qt>
#include <algorithm>
#include <cmath>

#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

CaptureViewElement::EventResult GlSlider::OnMouseEnter() {
  EventResult event_result = CaptureViewElement::OnMouseEnter();
  if (QGuiApplication::instance() != nullptr) {
    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
  }
  RequestUpdate(RequestUpdateScope::kDraw);
  return event_result;
}

CaptureViewElement::EventResult GlSlider::OnMouseLeave() {
  EventResult event_result = CaptureViewElement::OnMouseLeave();
  if (QGuiApplication::instance() != nullptr) {
    QGuiApplication::restoreOverrideCursor();
  }
  RequestUpdate(RequestUpdateScope::kDraw);
  return event_result;
}

CaptureViewElement::EventResult GlSlider::OnMouseMove(const Vec2& mouse_pos) {
  EventResult event_result = CaptureViewElement::OnMouseMove(mouse_pos);
  if (can_resize_ && QGuiApplication::instance() != nullptr) {
    if (PosIsInMinResizeArea(mouse_pos) || PosIsInMaxResizeArea(mouse_pos)) {
      QCursor cursor = is_vertical_ ? Qt::SizeVerCursor : Qt::SizeHorCursor;
      QGuiApplication::changeOverrideCursor(cursor);
    } else {
      QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
    }
  }
  RequestUpdate(RequestUpdateScope::kDraw);
  return event_result;
}

bool GlSlider::ContainsScreenSpacePoint(int x, int y) const {
  return x >= GetPos()[0] && x <= GetPos()[0] + GetSize()[0] && y >= GetPos()[1] &&
         y <= GetPos()[1] + GetSize()[1];
}

GlSlider::GlSlider(CaptureViewElement* parent, const Viewport* viewport,
                   const TimeGraphLayout* layout, TimelineInfoInterface* timeline_info,
                   bool is_vertical)
    : CaptureViewElement(parent, viewport, layout),
      is_vertical_(is_vertical),
      timeline_info_(timeline_info),
      pos_ratio_(0),
      right_edge_ratio_(0),
      length_ratio_(0),
      picking_pixel_offset_(0),
      selected_color_(75, 75, 75, 255),
      slider_color_(68, 68, 68, 255),
      bar_color_(61, 61, 61, 255) {}

void GlSlider::SetNormalizedPosition(float start_ratio)  // [0,1]
{
  pos_ratio_ = std::clamp(start_ratio, 0.f, 1.f);
  right_edge_ratio_ = pos_ratio_ * (1.0f - length_ratio_) + length_ratio_;
}

void GlSlider::SetNormalizedLength(float length_ratio)  // [0,1]
{
  float min_length = GetMinSliderPixelLength() / GetBarPixelLength();
  length_ratio_ = std::clamp(length_ratio, min_length, 1.f);
  right_edge_ratio_ = pos_ratio_ * (1.0f - length_ratio_) + length_ratio_;
}

Color GlSlider::GetLighterColor(const Color& color) {
  constexpr float kLocalGradientFactor = 1.0f + kGradientFactor;
  return {static_cast<unsigned char>(color[0] * kLocalGradientFactor),
          static_cast<unsigned char>(color[1] * kLocalGradientFactor),
          static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255};
}

Color GlSlider::GetDarkerColor(const Color& color) {
  constexpr float kLocalGradientFactor = 1.0f - kGradientFactor;
  return {static_cast<unsigned char>(color[0] * kLocalGradientFactor),
          static_cast<unsigned char>(color[1] * kLocalGradientFactor),
          static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255};
}

void GlSlider::OnDrag(int x, int y) {
  CaptureViewElement::OnDrag(x, y);
  Vec2 world_pos = viewport_->ScreenToWorld(Vec2i(x, y));
  float value = is_vertical_ ? world_pos[1] - GetPos()[1] : world_pos[0] - GetPos()[0];
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
                                 slider_right_pos - GetMinSliderPixelLength());
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
  CaptureViewElement::OnPick(x, y);
  Vec2 world_pos = viewport_->ScreenToWorld(Vec2i(x, y));
  float value = is_vertical_ ? world_pos[1] - GetPos()[1] : world_pos[0] - GetPos()[0];

  float slider_pos = PosToPixel(pos_ratio_);
  float slider_length = LenToPixel(length_ratio_);

  if (HandlePageScroll(value)) {
    drag_type_ = DragType::kNone;
    return;
  }

  if (can_resize_ && PosIsInMinResizeArea(world_pos)) {
    drag_type_ = DragType::kScaleMin;
    picking_pixel_offset_ = value - slider_pos;
  } else if (can_resize_ && PosIsInMaxResizeArea(world_pos)) {
    drag_type_ = DragType::kScaleMax;
    picking_pixel_offset_ = slider_pos + slider_length - value;
  } else {
    drag_type_ = DragType::kPan;
    picking_pixel_offset_ = value - slider_pos;
  }
}

void GlSlider::DrawBackground(PrimitiveAssembler& primitive_assembler, float x, float y,
                              float width, float height) {
  const Color dark_border_color = GetDarkerColor(bar_color_);
  constexpr float kEpsilon = 0.0001f;

  Quad border_box = MakeBox(Vec2(x, y), Vec2(width, height));
  primitive_assembler.AddBox(border_box, GlCanvas::kZValueButtonBg - kEpsilon, dark_border_color,
                             shared_from_this());

  Quad bar_box = MakeBox(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f));
  primitive_assembler.AddBox(bar_box, GlCanvas::kZValueButtonBg, bar_color_, shared_from_this());
}

void GlSlider::DrawSlider(PrimitiveAssembler& primitive_assembler, float x, float y, float width,
                          float height, ShadingDirection shading_direction) {
  bool is_picked = primitive_assembler.GetPickingManager()->IsThisElementPicked(this);

  Color color = (IsMouseOver() && PosIsInSlider(mouse_pos_cur_)) ? selected_color_ : slider_color_;
  const Color dark_border_color = GetDarkerColor(bar_color_);
  const Color light_border_color = GetLighterColor(color);
  constexpr float kEpsilon = 0.0001f;

  Quad dark_border_box = MakeBox(Vec2(x, y), Vec2(width, height));

  primitive_assembler.AddBox(dark_border_box, GlCanvas::kZValueButton - 2 * kEpsilon,
                             dark_border_color, shared_from_this());

  Quad light_border_box = MakeBox(Vec2(x + 1.f, y + 1.f), Vec2(width - 2.f, height - 2.f));

  primitive_assembler.AddBox(light_border_box, GlCanvas::kZValueButton - kEpsilon,
                             light_border_color, shared_from_this());

  // Slider itself
  constexpr float kSliderOffset = 2.f;
  if (!is_picked) {
    primitive_assembler.AddShadedBox(
        Vec2(x + kSliderOffset, y + kSliderOffset),
        Vec2(width - 2.f * kSliderOffset, height - 2.f * kSliderOffset), GlCanvas::kZValueButton,
        color, shared_from_this(), shading_direction);
  } else {
    primitive_assembler.AddBox(
        MakeBox(Vec2(x + kSliderOffset, y + kSliderOffset),
                Vec2(width - 2.f * kSliderOffset, height - 2.f * kSliderOffset)),
        GlCanvas::kZValueButton, slider_color_, shared_from_this());
  }
}

bool GlSlider::PosIsInMinResizeArea(const Vec2& pos) const {
  float relevant_value = is_vertical_ ? pos[1] - GetPos()[1] : pos[0] - GetPos()[0];
  return PosIsInSlider(pos) && relevant_value <= GetSliderPixelPos() + GetSliderResizeMargin();
}

bool GlSlider::PosIsInMaxResizeArea(const Vec2& pos) const {
  float relevant_value = is_vertical_ ? pos[1] - GetPos()[1] : pos[0] - GetPos()[0];
  return PosIsInSlider(pos) &&
         relevant_value >= GetSliderPixelPos() + GetSliderPixelLength() - GetSliderResizeMargin();
}

bool GlSlider::PosIsInSlider(const Vec2& pos) const {
  float relevant_value = is_vertical_ ? pos[1] - GetPos()[1] : pos[0] - GetPos()[0];
  return relevant_value >= GetSliderPixelPos() &&
         relevant_value <= GetSliderPixelPos() + GetSliderPixelLength();
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

void GlVerticalSlider::DoDraw(PrimitiveAssembler& primitive_assembler,
                              TextRenderer& /*text_renderer*/,
                              const DrawContext& /*draw_context*/) {
  // TODO(b/230442062): This should be part of CaptureViewElement.
  primitive_assembler.PushTranslation(static_cast<int>(GetPos()[0]), static_cast<int>(GetPos()[1]));

  float bar_pixel_len = GetBarPixelLength();
  float slider_height = std::ceil(length_ratio_ * bar_pixel_len);
  float non_slider_height = bar_pixel_len - slider_height;

  const Color dark_border_color = GetDarkerColor(bar_color_);

  // Background
  DrawBackground(primitive_assembler, 0, 0, GetSliderWidth(), bar_pixel_len);

  float start = std::ceil(pos_ratio_ * non_slider_height);

  ShadingDirection shading_direction = ShadingDirection::kRightToLeft;
  DrawSlider(primitive_assembler, 0, start, GetSliderWidth(), slider_height, shading_direction);

  primitive_assembler.PopTranslation();
}

float GlVerticalSlider::GetBarPixelLength() const { return GetHeight(); }

std::unique_ptr<orbit_accessibility::AccessibleInterface>
GlVerticalSlider::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureViewElement>(this, "Vertical Slider");
}

void GlHorizontalSlider::DoDraw(PrimitiveAssembler& primitive_assembler,
                                TextRenderer& /*text_renderer*/,
                                const DrawContext& /*draw_context*/) {
  // TODO(b/230442062): This should be part of CaptureViewElement.
  primitive_assembler.PushTranslation(static_cast<int>(GetPos()[0]), static_cast<int>(GetPos()[1]));

  float bar_pixel_len = GetBarPixelLength();
  float slider_width = std::ceil(length_ratio_ * bar_pixel_len);
  float non_slider_width = bar_pixel_len - slider_width;

  DrawBackground(primitive_assembler, 0, 0, bar_pixel_len, GetSliderWidth());

  float start = std::floor(pos_ratio_ * non_slider_width);

  ShadingDirection shading_direction = ShadingDirection::kTopToBottom;
  DrawSlider(primitive_assembler, start, 0, slider_width, GetSliderWidth(), shading_direction);

  constexpr float kEpsilon = 0.0001f;

  // Left / right resize arrows and separator
  constexpr float kHeightFactor = 2.f;
  // Triangle is 3 pixels smaller than the area - there is a 2 pixel border around the scrollbar,
  // and there is no margin between the border and the triangle to make it as big as possible.
  // On the other side, there is a 1 pixel margin between the triangle and the vertical line
  float tri_size = GetSliderResizeMargin() - 3.f;
  tri_size = std::min(tri_size, GetSliderWidth() - tri_size * kHeightFactor - 2.f);
  const float tri_y_offset = (GetSliderWidth() - tri_size * kHeightFactor) / 2.f;
  const Color white = GetLighterColor(GetLighterColor(bar_color_));
  const float z = GlCanvas::kZValueButton + 2 * kEpsilon;
  const float x = start;
  const float width = slider_width;

  primitive_assembler.AddTriangle(
      Triangle(Vec2(x + width - tri_size - 2.f, kHeightFactor * tri_size + tri_y_offset),
               Vec2(x + width - 2.f, tri_y_offset + kHeightFactor / 2.f * tri_size),
               Vec2(x + width - tri_size - 2.f, tri_y_offset)),
      z, white, shared_from_this());
  primitive_assembler.AddVerticalLine(Vec2(x + width - GetSliderResizeMargin(), 2.f),
                                      GetSliderWidth() - 4.f, z, white, shared_from_this());

  primitive_assembler.AddTriangle(
      Triangle(Vec2(x + tri_size + 2.f, kHeightFactor * tri_size + tri_y_offset),
               Vec2(x + tri_size + 2.f, tri_y_offset),
               Vec2(x + 2.f, tri_y_offset + kHeightFactor / 2.f * tri_size)),
      z, white, shared_from_this());
  primitive_assembler.AddVerticalLine(Vec2(x + GetSliderResizeMargin() + 1, 2.f),
                                      GetSliderWidth() - 4.f, z, white, shared_from_this());

  // Highlight the scale part of the slider
  bool is_picked = primitive_assembler.GetPickingManager()->IsThisElementPicked(this);
  if (is_picked) {
    if (drag_type_ == DragType::kScaleMax) {
      primitive_assembler.AddShadedBox(Vec2(x + width - GetSliderResizeMargin(), 2),
                                       Vec2(GetSliderResizeMargin() - 2, GetSliderWidth() - 4),
                                       GlCanvas::kZValueButton + kEpsilon, selected_color_,
                                       ShadingDirection::kTopToBottom);
    } else if (drag_type_ == DragType::kScaleMin) {
      primitive_assembler.AddShadedBox(
          Vec2(x + 2, 2), Vec2(GetSliderResizeMargin() - 2, GetSliderWidth() - 4),
          GlCanvas::kZValueButton + kEpsilon, selected_color_, ShadingDirection::kTopToBottom);
    }
  }

  primitive_assembler.PopTranslation();
}

float GlHorizontalSlider::GetBarPixelLength() const { return GetWidth(); }

std::unique_ptr<orbit_accessibility::AccessibleInterface>
GlHorizontalSlider::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureViewElement>(this, "Horizontal Slider");
}

}  // namespace orbit_gl