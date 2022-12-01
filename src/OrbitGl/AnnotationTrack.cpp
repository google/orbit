// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AnnotationTrack.h"

#include <GteVector.h>

#include <memory>

#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/PickingManager.h"

namespace {

const Color kWhite(255, 255, 255, 255);
const Color kFullyTransparent(255, 255, 255, 0);
const Color kThresholdColor(179, 0, 80, 255);

}  // namespace

using orbit_gl::PickingUserData;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

void AnnotationTrack::DrawAnnotation(PrimitiveAssembler& primitive_assembler,
                                     TextRenderer& text_renderer, const TimeGraphLayout* layout,
                                     int indentation_level, float z) {
  uint32_t font_size = GetAnnotationFontSize(indentation_level);
  Vec2 track_size = GetAnnotatedTrackSize();
  Vec2 track_pos = GetAnnotatedTrackPosition();

  float content_right_x = track_pos[0] + track_size[0];
  float content_bottom_y = track_pos[1] + track_size[1] - layout->GetTrackContentBottomMargin();
  float content_height = GetAnnotatedTrackContentHeight();

  // Add value upper bound text box (e.g., the "System Memory Total" text box for memory tracks).
  if (value_upper_bound_.has_value()) {
    std::string text = value_upper_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_position(content_right_x - string_width, content_bottom_y - content_height);
    text_renderer.AddText(text.c_str(), text_box_position[0], text_box_position[1], z,
                          {font_size, kWhite, string_width});

    if (!GetValueUpperBoundTooltip().empty()) {
      Vec2 text_box_size(string_width, layout->GetTextBoxHeight());
      auto user_data = std::make_unique<PickingUserData>(
          nullptr, [&](PickingId /*id*/) { return this->GetValueUpperBoundTooltip(); });
      primitive_assembler.AddShadedBox(text_box_position, text_box_size, z, kFullyTransparent,
                                       std::move(user_data));
    }
  }

  // Add value lower bound text box.
  if (value_lower_bound_.has_value()) {
    std::string text = value_lower_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_position(content_right_x - string_width, content_bottom_y);

    TextRenderer::TextFormatting formatting{
        font_size, kWhite, string_width, TextRenderer::HAlign::Left, TextRenderer::VAlign::Bottom};
    text_renderer.AddText(text.c_str(), text_box_position[0], text_box_position[1], z, formatting);
  }

  // Add warning threshold text box and line.
  if (warning_threshold_.has_value() && value_upper_bound_.has_value() &&
      value_lower_bound_.has_value()) {
    double min = value_lower_bound_.value().second;
    double max = value_upper_bound_.value().second;
    double warning_threshold = warning_threshold_.value().second;
    if (warning_threshold <= min || warning_threshold >= max) return;

    double normalized_value = (warning_threshold - min) / (max - min);
    float y = content_bottom_y - static_cast<float>(normalized_value) * content_height;

    std::string text = warning_threshold_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_position(track_pos[0] + layout->GetRightMargin(), y);

    TextRenderer::TextFormatting formatting{font_size, kThresholdColor, string_width,
                                            TextRenderer::HAlign::Left,
                                            TextRenderer::VAlign::Middle};
    text_renderer.AddText(text.c_str(), text_box_position[0], text_box_position[1], z, formatting);

    Vec2 from(track_pos[0], y);
    Vec2 to(track_pos[0] + track_size[0], y);
    primitive_assembler.AddLine(from, from + Vec2(layout->GetRightMargin() / 2.f, 0), z,
                                kThresholdColor);
    primitive_assembler.AddLine(Vec2(text_box_position[0] + string_width, y), to, z,
                                kThresholdColor);
  }
}