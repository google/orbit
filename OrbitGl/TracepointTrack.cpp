// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointTrack.h"

#include "App.h"

TracepointTrack::TracepointTrack(TimeGraph* time_graph, int32_t thread_id)
    : EventTrack(time_graph) {
  thread_id_ = thread_id;
}

void TracepointTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  if (IsEmpty()) {
    return;
  }

  Batcher* batcher = canvas->GetBatcher();

  float event_bar_z = picking_mode == PickingMode::kClick ? GlCanvas::kZValueEventBarPicking
                                                          : GlCanvas::kZValueEventBar;
  event_bar_z += z_offset;
  Color color = color_;
  Box box(pos_, Vec2(size_[0], -size_[1]), event_bar_z);
  batcher->AddBox(box, color, shared_from_this());

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = pos_[0];
  float y0 = pos_[1];
  float x1 = x0 + size_[0];
  float y1 = y0 - size_[1];

  batcher->AddLine(pos_, Vec2(x1, y0), event_bar_z, color, shared_from_this());
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), event_bar_z, color, shared_from_this());

  if (picked_) {
    Vec2& from = mouse_pos_[0];
    Vec2& to = mouse_pos_[1];

    x0 = from[0];
    y0 = pos_[1];
    x1 = to[0];
    y1 = y0 - size_[1];

    Color picked_color(0, 128, 255, 128);
    Box box(Vec2(x0, y0), Vec2(x1 - x0, -size_[1]), GlCanvas::kZValueUi + z_offset);
    batcher->AddBox(box, picked_color, shared_from_this());
  }

  canvas_ = canvas;
}

void TracepointTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick,
                                       PickingMode picking_mode, float z_offset) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float z = GlCanvas::kZValueEvent + z_offset;
  float track_height = layout.GetEventTrackHeight();
  const bool picking = picking_mode != PickingMode::kNone;

  const Color kWhite(255, 255, 255, 255);

  const Color kWhiteTransparent(255, 255, 255, 190);

  const Color kGrey(128, 128, 128, 255);

  const Color kGreenSelection(0, 255, 0, 255);

  if (!picking) {
    GOrbitApp->GetCaptureData().ForEachTracepointEventOfThreadInTimeRange(
        thread_id_, min_tick, max_tick,
        [&](const orbit_client_protos::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.time();
          float radius = track_height / 4;
          Vec2 pos(time_graph_->GetWorldFromTick(time), pos_[1]);
          if (thread_id_ == TracepointEventBuffer::kAllTracepointsFakeTid) {
            const Color color =
                tracepoint.pid() == GOrbitApp->GetCaptureData().process_id() ? kGrey : kWhite;
            batcher->AddVerticalLine(pos, -track_height, z, color);
          } else {
            batcher->AddVerticalLine(pos, -radius, z, kWhiteTransparent);
            batcher->AddVerticalLine(Vec2(pos[0], pos[1] - track_height), radius, z,
                                     kWhiteTransparent);
            batcher->AddCircle(Vec2(pos[0], pos[1] - track_height / 2), radius, z,
                               kWhiteTransparent);
          }
        });

  } else {
    constexpr float kPickingBoxWidth = 9.0f;
    constexpr float kPickingBoxOffset = kPickingBoxWidth / 2.0f;

    GOrbitApp->GetCaptureData().ForEachTracepointEventOfThreadInTimeRange(
        thread_id_, min_tick, max_tick,
        [&](const orbit_client_protos::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.time();
          Vec2 pos(time_graph_->GetWorldFromTick(time) - kPickingBoxOffset,
                   pos_[1] - track_height + 1);
          Vec2 size(kPickingBoxWidth, track_height);
          auto user_data = std::make_unique<PickingUserData>(
              nullptr, [&](PickingId id) -> std::string { return GetSampleTooltip(id); });
          user_data->custom_data_ = &tracepoint;
          batcher->AddShadedBox(pos, size, z, kGreenSelection, std::move(user_data));
        });
  }
}

void TracepointTrack::SetPos(float x, float y) { pos_ = Vec2(x, y); }

void TracepointTrack::OnPick(int x, int y) {
  Vec2& mouse_pos = mouse_pos_[0];
  canvas_->ScreenToWorld(x, y, mouse_pos[0], mouse_pos[1]);
  mouse_pos_[1] = mouse_pos_[0];
  picked_ = true;
}

void TracepointTrack::OnRelease() { picked_ = false; }

std::string TracepointTrack::GetSampleTooltip(PickingId id) const {
  auto user_data = time_graph_->GetBatcher().GetUserData(id);
  CHECK(user_data && user_data->custom_data_);

  const auto* tracepoint_event_info =
      static_cast<const orbit_client_protos::TracepointEventInfo*>(user_data->custom_data_);

  uint64_t tracepoint_info_key = tracepoint_event_info->tracepoint_info_key();

  TracepointInfo tracepoint_info =
      GOrbitApp->GetCaptureData().GetTracepointInfo(tracepoint_info_key);

  if (thread_id_ == TracepointEventBuffer::kAllTracepointsFakeTid) {
    return absl::StrFormat(
        "<b>%s : %s</b><br/>"
        "<i>Tracepoint event</i><br/>"
        "<br/>"
        "<b>Core:</b> %d<br/>"
        "<b>Process:</b> %s [%d]<br/>"
        "<b>Thread:</b> %s [%d]<br/>",
        tracepoint_info.category(), tracepoint_info.name(), tracepoint_event_info->cpu(),
        GOrbitApp->GetCaptureData().GetThreadName(tracepoint_event_info->pid()),
        tracepoint_event_info->pid(),
        GOrbitApp->GetCaptureData().GetThreadName(tracepoint_event_info->tid()),
        tracepoint_event_info->tid());
  } else {
    return absl::StrFormat(
        "<b>%s : %s</b><br/>"
        "<i>Tracepoint event</i><br/>"
        "<br/>"
        "<b>Core:</b> %d<br/>",
        tracepoint_info.category(), tracepoint_info.name(), tracepoint_event_info->cpu());
  }
}

bool TracepointTrack::IsEmpty() const {
  if (!GOrbitApp->HasCaptureData()) return true;
  return GOrbitApp->GetCaptureData().GetNumTracepointsForThreadId(thread_id_) == 0;
}
