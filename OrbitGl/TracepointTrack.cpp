// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointTrack.h"

#include "App.h"

TracepointTrack::TracepointTrack(TimeGraph* time_graph) : Track(time_graph) {
  color_ = Color(0, 255, 0, 255);
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
      Vec2 pos(time_graph_->GetWorldFromTick(time), position_[1]);
      batcher->AddVerticalLine(pos, -track_height, z, kWhite);
    } else {
      return;
    }
  }
}

void TracepointTrack::SetPos(float x, float y) {
  position_ = Vec2(x, y);
  thread_name_.SetPos(Vec2(x, y));
  thread_name_.SetSize(Vec2(size_[0] * 0.3f, size_[1]));
}