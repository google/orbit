// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Button.h"

#include "AccessibleButton.h"
#include "GlCanvas.h"

// TODO (b/230726102): Code below is copied from GlSlider. This should live in a central place.
namespace {
constexpr float kGradientFactor = 0.25f;

[[nodiscard]] Color GetLighterColor(const Color& color) {
  const float kLocalGradientFactor = 1.0f + kGradientFactor;
  return Color(static_cast<unsigned char>(color[0] * kLocalGradientFactor),
               static_cast<unsigned char>(color[1] * kLocalGradientFactor),
               static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255);
}

[[nodiscard]] Color GetDarkerColor(const Color& color) {
  const float kLocalGradientFactor = 1.0f - kGradientFactor;
  return Color(static_cast<unsigned char>(color[0] * kLocalGradientFactor),
               static_cast<unsigned char>(color[1] * kLocalGradientFactor),
               static_cast<unsigned char>(color[2] * kLocalGradientFactor), 255);
}
}  // namespace

namespace orbit_gl {

const Color Button::kHighlightColor(75, 75, 75, 255);
const Color Button::kBaseColor(68, 68, 68, 255);
const Color Button::kTextColor(255, 255, 255, 255);

Button::Button(CaptureViewElement* parent, const Viewport* viewport, const TimeGraphLayout* layout)
    : CaptureViewElement(parent, viewport, layout) {
  SetWidth(layout->GetMinButtonSize());
  SetHeight(layout->GetMinButtonSize());
}

void Button::SetHeight(float height) {
  if (height == height_) return;

  height_ = height;
  RequestUpdate();
}

void Button::SetLabel(const std::string& label) {
  if (label_ == label) return;

  label_ = label;
  RequestUpdate(RequestUpdateScope::kDraw);
}

void Button::SetMouseReleaseCallback(MouseReleaseCallback callback) {
  mouse_release_callback_ = std::move(callback);
}

void Button::OnRelease() {
  CaptureViewElement::OnRelease();
  if (mouse_release_callback_ != nullptr) {
    mouse_release_callback_(this);
  }
}

void Button::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  SetWidth(std::max(GetWidth(), layout_->GetMinButtonSize()));
  SetHeight(std::max(GetHeight(), layout_->GetMinButtonSize()));
}

void Button::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                    const DrawContext& /*draw_context*/) {
  const float z = GlCanvas::kZValueUi;
  const Vec2 pos = GetPos();
  const Vec2 size = GetSize();

  const Color kDarkBorderColor = GetDarkerColor(kBaseColor);
  const Color kLightBorderColor = GetLighterColor(kBaseColor);

  const Vec2 kBorderSize = Vec2(1.f, 1.f);

  Vec2 pos_w_border = pos;
  Vec2 size_w_border = size;

  // Dark border
  primitive_assembler.AddBox(MakeBox(pos_w_border, size_w_border), z, kDarkBorderColor);
  pos_w_border += kBorderSize;
  size_w_border -= kBorderSize + kBorderSize;

  // Light border
  primitive_assembler.AddBox(MakeBox(pos_w_border, size_w_border), z, kLightBorderColor);
  pos_w_border += kBorderSize;
  size_w_border -= kBorderSize + kBorderSize;

  // Button itself
  primitive_assembler.AddShadedBox(pos_w_border, size_w_border, z, kBaseColor,
                                   ShadingDirection::kTopToBottom);

  TextRendererInterface::TextFormatting format;
  format.color = kTextColor;
  format.valign = TextRendererInterface::VAlign::Middle;
  format.halign = TextRendererInterface::HAlign::Centered;
  format.max_size = GetSize()[0];
  format.font_size = layout_->CalculateZoomedFontSize();

  const float x = pos[0] + size[0] / 2.f;
  const float y = pos[1] + size[1] / 2.f;
  text_renderer.AddText(label_.c_str(), x, y, z, format);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Button::CreateAccessibleInterface() {
  return std::make_unique<AccessibleButton>(this);
}

}  // namespace orbit_gl