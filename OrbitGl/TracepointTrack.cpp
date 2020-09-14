// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointTrack.h"

#include "App.h"

TracepointTrack::TracepointTrack(TimeGraph* time_graph) : EventTrack(time_graph) {}

void TracepointTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  if (!were_tracepoints_hit_per_thread_) {
    const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& tracepoints =
        GOrbitApp->GetCaptureData().GetTracepointsOfThread(thread_id_);

    if (!tracepoints.empty()) {
      were_tracepoints_hit_per_thread_ = true;
    }
  }

  if (!were_tracepoints_hit_per_thread_) {
    return;
  }
  Batcher* batcher = canvas->GetBatcher();

  const float eventBarZ = picking_mode == PickingMode::kClick ? GlCanvas::kZValueEventBarPicking
                                                              : GlCanvas::kZValueEventBar;
  Color color = color_;
  Box box(pos_, Vec2(size_[0], -size_[1]), eventBarZ);
  batcher->AddBox(box, color, shared_from_this());

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = pos_[0];
  float y0 = pos_[1];
  float x1 = x0 + size_[0];
  float y1 = y0 - size_[1];

  batcher->AddLine(pos_, Vec2(x1, y0), GlCanvas::kZValueEventBar, color, shared_from_this());
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), GlCanvas::kZValueEventBar, color,
                   shared_from_this());

  canvas_ = canvas;
}

void TracepointTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float z = GlCanvas::kZValueEvent;
  float track_height = layout.GetEventTrackHeight();

  ScopeLock lock(GOrbitApp->GetCaptureData().GetTracepointEventBufferMutex());

  const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& tracepoints =
      GOrbitApp->GetCaptureData().GetTracepointsOfThread(thread_id_);

  const Color kWhite(255, 255, 255, 255);

  for (auto it = tracepoints.lower_bound(min_tick); it != tracepoints.upper_bound(max_tick); ++it) {
    uint64_t time = it->first;
    if (time < max_tick) {
      Vec2 pos(time_graph_->GetWorldFromTick(time), pos_[1]);
      batcher->AddVerticalLine(pos, -track_height, z, kWhite);
    } else {
      return;
    }
  }
}

void TracepointTrack::SetPos(float x, float y) {
  y = y - GetHeight();
  pos_ = Vec2(x, y);

  thread_name_.SetPos(pos_);
  thread_name_.SetSize(Vec2(size_[0] * 0.3f, size_[1]));
}

float TracepointTrack::GetEventTrackHeightAndExtraSpace() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();

  return were_tracepoints_hit_per_thread_ == true
             ? layout.GetEventTrackHeight() + layout.GetSpaceBetweenTracksAndThread()
             : 0;
}

float TracepointTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();

  return were_tracepoints_hit_per_thread_ == true
             ? GetEventTrackHeightAndExtraSpace() + layout.GetTrackBottomMargin()
             : 0;
}
