// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphTrack.h"

#include "GlCanvas.h"

GraphTrack::GraphTrack(TimeGraph* time_graph, std::string name)
    : Track(time_graph), name_(std::move(name)) {}

void GraphTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  Batcher* batcher = canvas->GetBatcher();

  float trackWidth = canvas->GetWorldWidth();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const bool picking = picking_mode != PickingMode::kNone;

  pos_[0] = canvas->GetWorldTopLeftX();
  SetSize(trackWidth, GetHeight());
  Track::Draw(canvas, picking_mode);

  float x0 = pos_[0];
  float x1 = x0 + size_[0];

  float y0 = pos_[1];
  float y1 = y0 - size_[1];

  const Color kPickedColor(0, 128, 255, 128);
  Color color = GetBackGroundColor();

  float track_z = GlCanvas::kZValueTrack;

  Box box(pos_, Vec2(size_[0], -size_[1]), track_z);

  // Draw label with graph value at current mouse position.
  const Color kWhiteColor(255, 255, 255, 255);
  const Color kBlackColor(0, 0, 0, 255);
  double graph_value = GetValueAtTime(time_graph_->GetCurrentMouseTimeNs());

  int white_text_box_width =
      canvas->GetTextRenderer().GetStringWidth(std::to_string(graph_value).c_str());

  Vec2 white_text_box_size(white_text_box_width, layout.GetTextBoxHeight());
  auto rightmost_x_position = canvas->GetWorldTopLeftX() + canvas->GetWorldWidth() -
                              layout.GetRightMargin() - layout.GetSliderWidth() -
                              white_text_box_size[0];
  Vec2 white_text_box_position(
      std::min(canvas->GetMouseX(), rightmost_x_position),
      y0 - static_cast<float>((max_ - graph_value) * size_[1] / value_range_) -
          white_text_box_size[1] / 2.f);

  if (!picking) {
    canvas->GetTextRenderer().AddText(
        std::to_string(graph_value).c_str(), white_text_box_position[0],
        white_text_box_position[1] + layout.GetTextOffset(), GlCanvas::kZValueTextUi, kBlackColor,
        time_graph_->CalculateZoomedFontSize(), white_text_box_size[0]);
    Box white_text_box(white_text_box_position, white_text_box_size, GlCanvas::kZValueUi);
    batcher->AddBox(white_text_box, kWhiteColor);
  }

  batcher->AddBox(box, color, shared_from_this());

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  batcher->AddLine(pos_, Vec2(x1, y0), track_z, color, shared_from_this());
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), track_z, color, shared_from_this());

  const Color kLineColor(0, 128, 255, 128);

  if (!picking) {
    // Current time window
    uint64_t min_ns = time_graph_->GetTickFromUs(time_graph_->GetMinTimeUs());
    uint64_t max_ns = time_graph_->GetTickFromUs(time_graph_->GetMaxTimeUs());
    double time_range = static_cast<float>(max_ns - min_ns);
    if (values_.size() < 2 || time_range == 0) return;

    auto it = values_.upper_bound(min_ns);
    if (it == values_.end()) return;
    if (it != values_.begin()) --it;
    uint64_t previous_time = it->first;
    double last_normalized_value = (it->second - min_) * inv_value_range_;
    for (++it; it != values_.end(); ++it) {
      if (previous_time > max_ns) break;
      uint64_t time = it->first;
      double normalized_value = (it->second - min_) * inv_value_range_;
      float base_y = pos_[1] - size_[1];
      float x0 = time_graph_->GetWorldFromTick(previous_time);
      float x1 = time_graph_->GetWorldFromTick(time);
      float y0 = base_y + static_cast<float>(last_normalized_value) * size_[1];
      float y1 = base_y + static_cast<float>(normalized_value) * size_[1];
      time_graph_->GetBatcher().AddLine(Vec2(x0, y0), Vec2(x1, y0), GlCanvas::kZValueText,
                                        kLineColor);
      time_graph_->GetBatcher().AddLine(Vec2(x1, y0), Vec2(x1, y1), GlCanvas::kZValueText,
                                        kLineColor);

      previous_time = time;
      last_normalized_value = normalized_value;
    }
  }
}

void GraphTrack::AddValue(double value, uint64_t time) {
  values_[time] = value;
  max_ = std::max(max_, value);
  min_ = std::min(min_, value);
  value_range_ = max_ - min_;

  if (value_range_ > 0) inv_value_range_ = 1.0 / value_range_;
}

double GraphTrack::GetValueAtTime(uint64_t time, double default_value) const {
  auto iterator_lower = values_.upper_bound(time);
  if (iterator_lower == values_.end() || iterator_lower == values_.begin()) {
    return default_value;
  }
  --iterator_lower;
  return iterator_lower->second;
}

float GraphTrack::GetHeight() const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float height = layout.GetTextBoxHeight() + layout.GetSpaceBetweenTracksAndThread() +
                 layout.GetEventTrackHeight() + layout.GetTrackBottomMargin();
  return height;
}
