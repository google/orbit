// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTrack.h"

#include <algorithm>

#include "GlCanvas.h"

namespace orbit_gl {

void MemoryTrack::Draw(Batcher& batcher, TextRenderer& text_renderer,
                       uint64_t current_mouse_time_ns, PickingMode picking_mode, float z_offset) {
  GraphTrack::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);

  if (values_.empty() || picking_mode != PickingMode::kNone) return;

  float text_z = GlCanvas::kZValueTrackText + z_offset;
  uint32_t font_size = layout_->CalculateZoomedFontSize();

  float content_height =
      (size_[1] - layout_->GetTrackTabHeight() - layout_->GetTrackBottomMargin());
  Vec2 content_pos = pos_;
  content_pos[1] -= layout_->GetTrackTabHeight();

  // Add warning threshold text box and line.
  if (warning_threshold_.has_value()) {
    const Color kThresholdColor(244, 67, 54, 255);
    double normalized_value = (warning_threshold_.value().second - min_) * inv_value_range_;
    float x = pos_[0];
    float y =
        content_pos[1] - content_height + static_cast<float>(normalized_value) * content_height;
    Vec2 from(x, y);
    Vec2 to(x + size_[0], y);

    std::string text = warning_threshold_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout_->GetTextBoxHeight());
    Vec2 text_box_position(pos_[0] + layout_->GetRightMargin(),
                           y - layout_->GetTextBoxHeight() / 2.f);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout_->GetTextOffset(), text_z, kThresholdColor,
                          font_size, text_box_size[0]);

    batcher.AddLine(from, from + Vec2(layout_->GetRightMargin() / 2.f, 0), text_z, kThresholdColor);
    batcher.AddLine(Vec2(text_box_position[0] + text_box_size[0], y), to, text_z, kThresholdColor);
  }

  // Add value upper bound text box (e.g., the "System Memory Total" text box for memory tracks).
  const Color kWhite(255, 255, 255, 255);
  if (value_upper_bound_.has_value()) {
    std::string text = value_upper_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout_->GetTextBoxHeight());
    Vec2 text_box_position(pos_[0] + size_[0] - text_box_size[0] - layout_->GetRightMargin() -
                               layout_->GetSliderWidth(),
                           content_pos[1] - layout_->GetTextBoxHeight() / 2.f);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout_->GetTextOffset(), text_z, kWhite,
                          font_size, text_box_size[0]);
  }

  // Add value lower bound text box.
  if (value_lower_bound_.has_value()) {
    std::string text = value_lower_bound_.value().first;
    float string_width = text_renderer.GetStringWidth(text.c_str(), font_size);
    Vec2 text_box_size(string_width, layout_->GetTextBoxHeight());
    Vec2 text_box_position(pos_[0] + size_[0] - text_box_size[0] - layout_->GetRightMargin() -
                               layout_->GetSliderWidth(),
                           content_pos[1] - content_height);
    text_renderer.AddText(text.c_str(), text_box_position[0],
                          text_box_position[1] + layout_->GetTextOffset(), text_z, kWhite,
                          font_size, text_box_size[0]);
  }
}

void MemoryTrack::SetWarningThresholdWhenEmpty(const std::string& pretty_label, double raw_value) {
  if (warning_threshold_.has_value()) return;
  warning_threshold_ = std::make_pair(pretty_label, raw_value);
  UpdateMinAndMax(raw_value);
}

void MemoryTrack::SetValueUpperBoundWhenEmpty(const std::string& pretty_label, double raw_value) {
  if (value_upper_bound_.has_value()) return;
  value_upper_bound_ = std::make_pair(pretty_label, raw_value);
  UpdateMinAndMax(raw_value);
}
void MemoryTrack::SetValueLowerBoundWhenEmpty(const std::string& pretty_label, double raw_value) {
  if (value_lower_bound_.has_value()) return;
  value_lower_bound_ = std::make_pair(pretty_label, raw_value);
  UpdateMinAndMax(raw_value);
}

void MemoryTrack::UpdateMinAndMax(double value) {
  max_ = std::max(max_, value);
  min_ = std::min(min_, value);
}

}  // namespace orbit_gl