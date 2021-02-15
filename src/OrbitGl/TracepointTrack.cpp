// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <memory>
#include <utility>

#include "App.h"
#include "Batcher.h"
#include "CoreMath.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitClientModel/CaptureData.h"
#include "OrbitClientServices/TracepointServiceClient.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "capture_data.pb.h"
#include "tracepoint.pb.h"

namespace orbit_gl {

TracepointTrack::TracepointTrack(OrbitApp* app, TimeGraph* time_graph, TimeGraphLayout* layout,
                                 const CaptureData* capture_data, int32_t thread_id)
    : ThreadBar(app, time_graph, layout, capture_data, thread_id), color_{255, 0, 0, 255} {}

void TracepointTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  ThreadBar::Draw(canvas, picking_mode, z_offset);

  if (IsEmpty()) {
    return;
  }

  Batcher* ui_batcher = canvas->GetBatcher();

  float event_bar_z = picking_mode == PickingMode::kClick ? GlCanvas::kZValueEventBarPicking
                                                          : GlCanvas::kZValueEventBar;
  event_bar_z += z_offset;
  Color color = color_;
  Box box(pos_, Vec2(size_[0], -size_[1]), event_bar_z);
  ui_batcher->AddBox(box, color, shared_from_this());
}

void TracepointTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                       PickingMode picking_mode, float z_offset) {
  ThreadBar::UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);

  float z = GlCanvas::kZValueEvent + z_offset;
  float track_height = layout_->GetEventTrackHeight();
  const bool picking = picking_mode != PickingMode::kNone;

  const Color kWhite(255, 255, 255, 255);
  const Color kWhiteTransparent(255, 255, 255, 190);
  const Color kGrey(128, 128, 128, 255);

  CHECK(capture_data_ != nullptr);

  if (!picking) {
    capture_data_->ForEachTracepointEventOfThreadInTimeRange(
        thread_id_, min_tick, max_tick,
        [&](const orbit_client_protos::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.time();
          float radius = track_height / 4;
          Vec2 pos(time_graph_->GetWorldFromTick(time), pos_[1]);
          if (thread_id_ == orbit_base::kAllThreadsOfAllProcessesTid) {
            const Color color = tracepoint.pid() == capture_data_->process_id() ? kGrey : kWhite;
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

    capture_data_->ForEachTracepointEventOfThreadInTimeRange(
        thread_id_, min_tick, max_tick,
        [&](const orbit_client_protos::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.time();
          Vec2 pos(time_graph_->GetWorldFromTick(time) - kPickingBoxOffset,
                   pos_[1] - track_height + 1);
          Vec2 size(kPickingBoxWidth, track_height);
          auto user_data =
              std::make_unique<PickingUserData>(nullptr, [&, batcher](PickingId id) -> std::string {
                return GetTracepointTooltip(batcher, id);
              });
          user_data->custom_data_ = &tracepoint;
          batcher->AddShadedBox(pos, size, z, kWhite, std::move(user_data));
        });
  }
}

std::string TracepointTrack::GetTracepointTooltip(Batcher* batcher, PickingId id) const {
  auto user_data = batcher->GetUserData(id);
  CHECK(user_data && user_data->custom_data_);

  const auto* tracepoint_event_info =
      static_cast<const orbit_client_protos::TracepointEventInfo*>(user_data->custom_data_);

  uint64_t tracepoint_info_key = tracepoint_event_info->tracepoint_info_key();

  CHECK(capture_data_ != nullptr);

  TracepointInfo tracepoint_info = capture_data_->GetTracepointInfo(tracepoint_info_key);

  if (thread_id_ == orbit_base::kAllThreadsOfAllProcessesTid) {
    return absl::StrFormat(
        "<b>%s : %s</b><br/>"
        "<i>Tracepoint event</i><br/>"
        "<br/>"
        "<b>Core:</b> %d<br/>"
        "<b>Process:</b> %s [%d]<br/>"
        "<b>Thread:</b> %s [%d]<br/>",
        tracepoint_info.category(), tracepoint_info.name(), tracepoint_event_info->cpu(),
        capture_data_->GetThreadName(tracepoint_event_info->pid()), tracepoint_event_info->pid(),
        capture_data_->GetThreadName(tracepoint_event_info->tid()), tracepoint_event_info->tid());
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
  if (capture_data_ == nullptr) return true;
  return capture_data_->GetNumTracepointsForThreadId(thread_id_) == 0;
}

}  // namespace orbit_gl