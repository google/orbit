// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AnnotationTrack.h"

namespace {

const Color kWhite(255, 255, 255, 255);
const Color kThresholdColor(244, 67, 54, 255);

}  // namespace

void AnnotationTrack::DrawAnnotation(Batcher& batcher, TextRenderer& text_renderer,
                                     TimeGraphLayout* layout, float z) {
  uint32_t font_size = layout->CalculateZoomedFontSize();
  Vec2 track_size = GetAnnotatedTrackSize();
  Vec2 track_pos = GetAnnotatedTrackPosition();

  float content_right_x =
      track_pos[0] + track_size[0] - layout->GetRightMargin() - layout->GetSliderWidth();
  float content_bottom_y = track_pos[1] - track_size[1] + layout->GetTrackBottomMargin();
  float content_height = GetAnnotatedTrackContentHeight();

  // Add value upper bound text box (e.g., the "System Memory Total" text box for memory tracks).
  if (value_upper_bound_.has_value()) {
    std::string text = value_upper_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout->GetTextBoxHeight());
    Vec2 text_box_position(content_right_x - text_box_size[0],
                           content_bottom_y + content_height - text_box_size[1]);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout->GetTextOffset(), z, kWhite, font_size,
                          text_box_size[0]);
  }

  // Add value lower bound text box.
  if (value_lower_bound_.has_value()) {
    std::string text = value_lower_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout->GetTextBoxHeight());
    Vec2 text_box_position(content_right_x - text_box_size[0], content_bottom_y);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout->GetTextOffset(), z, kWhite, font_size,
                          text_box_size[0]);
  }

  // Add warning threshold text box and line.
  if (warning_threshold_.has_value() && value_upper_bound_.has_value() &&
      value_lower_bound_.has_value()) {
    double min = value_lower_bound_.value().second;
    double max = value_upper_bound_.value().second;
    double warning_threshold = warning_threshold_.value().second;
    if (warning_threshold <= min || warning_threshold >= max) return;

    double normalized_value = (warning_threshold - min) / (max - min);
    float y = content_bottom_y + static_cast<float>(normalized_value) * content_height;

    std::string text = warning_threshold_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout->GetTextBoxHeight());
    Vec2 text_box_position(track_pos[0] + layout->GetRightMargin(), y - text_box_size[1] / 2.f);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout->GetTextOffset(), z, kThresholdColor,
                          font_size, text_box_size[0]);

    Vec2 from(track_pos[0], y);
    Vec2 to(track_pos[0] + track_size[0], y);
    batcher.AddLine(from, from + Vec2(layout->GetRightMargin() / 2.f, 0), z, kThresholdColor);
    batcher.AddLine(Vec2(text_box_position[0] + text_box_size[0], y), to, z, kThresholdColor);
  }
}