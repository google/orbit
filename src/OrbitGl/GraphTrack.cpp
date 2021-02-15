// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphTrack.h"

#include <GteVector.h>

#include <algorithm>
#include <cstdint>

#include "Geometry.h"
#include "GlCanvas.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"

GraphTrack::GraphTrack(TimeGraph* time_graph, TimeGraphLayout* layout, std::string name,
                       CaptureData* capture_data)
    : Track(time_graph, layout, capture_data) {
  SetName(name);
  SetLabel(name);
}

void GraphTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                  PickingMode picking_mode, float z_offset) {
  GlCanvas* canvas = time_graph_->GetCanvas();

  float track_width = canvas->GetWorldWidth();
  SetSize(track_width, GetHeight());
  pos_[0] = canvas->GetWorldTopLeftX();

  Color color = GetBackgroundColor();
  const Color kLineColor(0, 128, 255, 128);
  const Color kDotColor(0, 128, 255, 255);
  float track_z = GlCanvas::kZValueTrack + z_offset;
  float graph_z = GlCanvas::kZValueEventBar + z_offset;
  float dot_z = GlCanvas::kZValueBox + z_offset;

  Box box(pos_, Vec2(size_[0], -size_[1]), track_z);
  batcher->AddBox(box, color, shared_from_this());

  const bool picking = picking_mode != PickingMode::kNone;
  if (!picking) {
    double time_range = static_cast<double>(max_tick - min_tick);
    if (values_.size() < 2 || time_range == 0) return;

    auto it = values_.upper_bound(min_tick);
    if (it == values_.end()) return;
    if (it != values_.begin()) --it;
    uint64_t previous_time = it->first;
    double last_normalized_value = (it->second - min_) * inv_value_range_;
    constexpr float kDotRadius = 2.f;
    float base_y = pos_[1] - size_[1];
    float y1 = 0;
    DrawSquareDot(batcher,
                  Vec2(time_graph_->GetWorldFromTick(previous_time),
                       base_y + static_cast<float>(last_normalized_value) * size_[1]),
                  kDotRadius, dot_z, kDotColor);

    for (++it; it != values_.end(); ++it) {
      if (previous_time > max_tick) break;
      uint64_t time = it->first;
      double normalized_value = (it->second - min_) * inv_value_range_;
      float x0 = time_graph_->GetWorldFromTick(previous_time);
      float x1 = time_graph_->GetWorldFromTick(time);
      float y0 = base_y + static_cast<float>(last_normalized_value) * size_[1];
      y1 = base_y + static_cast<float>(normalized_value) * size_[1];
      batcher->AddLine(Vec2(x0, y0), Vec2(x1, y0), graph_z, kLineColor);
      batcher->AddLine(Vec2(x1, y0), Vec2(x1, y1), graph_z, kLineColor);
      DrawSquareDot(batcher, Vec2(x1, y1), kDotRadius, dot_z, kDotColor);

      previous_time = time;
      last_normalized_value = normalized_value;
    }

    if (!values_.empty()) {
      float x0 = time_graph_->GetWorldFromTick(previous_time);
      float x1 = time_graph_->GetWorldFromTick(max_tick);
      batcher->AddLine(Vec2(x0, y1), Vec2(x1, y1), graph_z, kLineColor);
    }
  }
}

void GraphTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  Track::Draw(canvas, picking_mode, z_offset);
  if (values_.empty() || picking_mode != PickingMode::kNone) {
    return;
  }

  // Draw label
  uint64_t current_mouse_time_ns = time_graph_->GetCurrentMouseTimeNs();
  auto previous_point = GetPreviousValueAndTime(current_mouse_time_ns);

  double value =
      previous_point.has_value() ? previous_point.value().second : values_.begin()->second;
  uint64_t first_time = values_.begin()->first;
  uint64_t label_time = std::max(current_mouse_time_ns, first_time);
  float point_x = time_graph_->GetWorldFromTick(label_time);
  double normalized_value = (value - min_) * inv_value_range_;
  float point_y = pos_[1] - size_[1] * (1.f - static_cast<float>(normalized_value));
  const Color kBlack(0, 0, 0, 255);
  const Color kWhite(255, 255, 255, 255);
  float text_z = GlCanvas::kZValueEvent + z_offset;
  DrawLabel(canvas, Vec2(point_x, point_y), std::to_string(value), kBlack, kWhite, text_z);
}

void GraphTrack::DrawSquareDot(Batcher* batcher, Vec2 center, float radius, float z,
                               const Color& color) {
  Vec2 position(center[0] - radius, center[1] - radius);
  Vec2 size(2 * radius, 2 * radius);
  batcher->AddBox(Box(position, size, z), color);
}

void GraphTrack::DrawLabel(GlCanvas* canvas, Vec2 target_pos, const std::string& text,
                           const Color& text_color, const Color& font_color, float z) {
  uint32_t font_size = time_graph_->CalculateZoomedFontSize();

  float text_width = canvas->GetTextRenderer().GetStringWidth(text.c_str(), font_size);
  Vec2 text_box_size(text_width, layout_->GetTextBoxHeight());

  float arrow_width = text_box_size[1] / 2.f;
  bool arrow_is_left_directed =
      target_pos[0] < canvas->GetWorldTopLeftX() + text_box_size[0] + arrow_width;
  Vec2 text_box_position(
      target_pos[0] + (arrow_is_left_directed ? arrow_width : -arrow_width - text_box_size[0]),
      target_pos[1] - text_box_size[1] / 2.f);

  Box arrow_text_box(text_box_position, text_box_size, z);
  Vec3 arrow_extra_point(target_pos[0], target_pos[1], z);

  Batcher* ui_batcher = canvas->GetBatcher();
  ui_batcher->AddBox(arrow_text_box, font_color);
  if (arrow_is_left_directed) {
    ui_batcher->AddTriangle(
        Triangle(arrow_text_box.vertices[0], arrow_text_box.vertices[1], arrow_extra_point),
        font_color);
  } else {
    ui_batcher->AddTriangle(
        Triangle(arrow_text_box.vertices[2], arrow_text_box.vertices[3], arrow_extra_point),
        font_color);
  }

  canvas->GetTextRenderer().AddText(text.c_str(), text_box_position[0],
                                    text_box_position[1] + layout_->GetTextOffset(), z, text_color,
                                    font_size, text_box_size[0]);
}

void GraphTrack::AddValue(double value, uint64_t time) {
  values_[time] = value;
  max_ = std::max(max_, value);
  min_ = std::min(min_, value);
  value_range_ = max_ - min_;

  if (value_range_ > 0) inv_value_range_ = 1.0 / value_range_;
}

std::optional<std::pair<uint64_t, double> > GraphTrack::GetPreviousValueAndTime(
    uint64_t time) const {
  auto iterator_lower = values_.upper_bound(time);
  if (iterator_lower == values_.begin()) {
    return {};
  }
  --iterator_lower;
  return *iterator_lower;
}

float GraphTrack::GetHeight() const {
  float height = layout_->GetTextBoxHeight() + layout_->GetSpaceBetweenTracksAndThread() +
                 layout_->GetEventTrackHeight() + layout_->GetTrackBottomMargin();
  return height;
}
