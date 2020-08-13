// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphTrack.h"

#include "GlCanvas.h"

GraphTrack::GraphTrack(TimeGraph* time_graph, uint64_t graph_id)
    : Track(time_graph), graph_id_(graph_id) {}

void GraphTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  Track::Draw(canvas, picking_mode);
  Batcher* batcher = canvas->GetBatcher();

  TimeGraphLayout& layout = time_graph_->GetLayout();
  float trackWidth = canvas->GetWorldWidth();

  m_Pos[0] = canvas->GetWorldTopLeftX();
  SetSize(trackWidth, GetHeight());

  float x0 = m_Pos[0];
  float x1 = x0 + m_Size[0];

  float y0 = m_Pos[1];
  float y1 = y0 - m_Size[1];

  const Color kPickedColor(0, 128, 255, 128);
  Color color = m_Color;
  if (m_Picked) {
    // TODO: Is this really used? Is m_Picked even a thing?
    color = kPickedColor;
  }

  float track_z = layout.GetTrackZ();
  float text_z = layout.GetTextZ();

  Box box(m_Pos, Vec2(m_Size[0], -m_Size[1]), track_z);
  batcher->AddBox(box, color, shared_from_this());

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  batcher->AddLine(m_Pos, Vec2(x1, y0), track_z, color, shared_from_this());
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), track_z, color,
                   shared_from_this());

  const Color kLineColor(0, 128, 255, 128);

  // Current time window
  uint64_t min_ns = time_graph_->GetTickFromUs(time_graph_->GetMinTimeUs());
  uint64_t max_ns = time_graph_->GetTickFromUs(time_graph_->GetMaxTimeUs());
  double time_range = static_cast<float>(max_ns - min_ns);
  if (values_.size() < 2 || time_range == 0) return;

  auto it = values_.lower_bound(min_ns);
  if (it == values_.end()) return;
  uint64_t previous_time = it->first;
  double last_normalized_value = (it->second - min_) * inv_value_range_;
  for (++it; it != values_.end(); ++it) {
    if (previous_time > max_ns) break;
    uint64_t time = it->first;
    double normalized_value = (it->second - min_) * inv_value_range_;
    float base_y = m_Pos[1] - m_Size[1];
    float x0 = time_graph_->GetWorldFromTick(previous_time);
    float x1 = time_graph_->GetWorldFromTick(time);
    float y0 = base_y + static_cast<float>(last_normalized_value) * m_Size[1];
    float y1 = base_y + static_cast<float>(normalized_value) * m_Size[1];
    time_graph_->GetBatcher().AddLine(Vec2(x0, y0), Vec2(x1, y1), text_z,
                                      kLineColor);

    previous_time = time;
    last_normalized_value = normalized_value;
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
  auto iterator_lower = values_.lower_bound(time);
  if (iterator_lower != values_.end()) {
    return iterator_lower->second;
  }
  auto iterator_upper = values_.upper_bound(time);
  if (iterator_upper != values_.end()) {
    return iterator_upper->second;
  }
  return default_value;
}

float GraphTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  float height = layout.GetTextBoxHeight() +
                 layout.GetSpaceBetweenTracksAndThread() +
                 layout.GetEventTrackHeight() + layout.GetTrackBottomMargin();
  return height;
}
