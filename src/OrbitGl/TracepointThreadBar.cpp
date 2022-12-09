// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TracepointThreadBar.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <memory>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/TracepointEventInfo.h"
#include "ClientData/TracepointInfo.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/ThreadColor.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

TracepointThreadBar::TracepointThreadBar(CaptureViewElement* parent, OrbitApp* app,
                                         const orbit_gl::TimelineInfoInterface* timeline_info,
                                         orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                         const orbit_client_data::ModuleManager* module_manager,
                                         const orbit_client_data::CaptureData* capture_data,
                                         uint32_t thread_id)
    : ThreadBar(parent, app, timeline_info, viewport, layout, module_manager, capture_data,
                thread_id, "Tracepoints") {}

void TracepointThreadBar::DoDraw(PrimitiveAssembler& primitive_assembler,
                                 TextRenderer& text_renderer, const DrawContext& draw_context) {
  ThreadBar::DoDraw(primitive_assembler, text_renderer, draw_context);

  if (IsEmpty()) {
    return;
  }

  float event_bar_z = draw_context.picking_mode == PickingMode::kClick
                          ? GlCanvas::kZValueEventBarPicking
                          : GlCanvas::kZValueEventBar;
  Color color = orbit_gl::GetThreadColor(GetThreadId());
  Quad box = MakeBox(GetPos(), Vec2(GetWidth(), -GetHeight()));
  primitive_assembler.AddBox(box, event_bar_z, color, shared_from_this());
}

void TracepointThreadBar::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                             TextRenderer& text_renderer, uint64_t min_tick,
                                             uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("TracepointThreadBar::DoUpdatePrimitives", kOrbitColorIndigo);
  ThreadBar::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                picking_mode);

  float z = GlCanvas::kZValueEvent;
  float track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const bool picking = picking_mode != PickingMode::kNone;

  const Color white(255, 255, 255, 255);
  const Color white_transparent(255, 255, 255, 190);
  const Color grey(128, 128, 128, 255);

  ORBIT_CHECK(capture_data_ != nullptr);

  if (!picking) {
    capture_data_->ForEachTracepointEventOfThreadInTimeRange(
        GetThreadId(), min_tick, max_tick,
        [&](const orbit_client_data::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.timestamp_ns();
          float radius = track_height / 4;
          const Vec2 pos(timeline_info_->GetWorldFromTick(time), GetPos()[1]);
          if (GetThreadId() == orbit_base::kAllThreadsOfAllProcessesTid) {
            const Color color = tracepoint.pid() == capture_data_->process_id() ? grey : white;
            primitive_assembler.AddVerticalLine(pos, -track_height, z, color);
          } else {
            primitive_assembler.AddVerticalLine(pos, -radius, z, white_transparent);
            primitive_assembler.AddVerticalLine(Vec2(pos[0], pos[1] - track_height), radius, z,
                                                white_transparent);
            primitive_assembler.AddCircle(Vec2(pos[0], pos[1] - track_height / 2), radius, z,
                                          white_transparent);
          }
        });

  } else {
    constexpr float kPickingBoxWidth = 9.0f;
    constexpr float kPickingBoxOffset = kPickingBoxWidth / 2.0f;

    capture_data_->ForEachTracepointEventOfThreadInTimeRange(
        GetThreadId(), min_tick, max_tick,
        [&](const orbit_client_data::TracepointEventInfo& tracepoint) {
          uint64_t time = tracepoint.timestamp_ns();
          Vec2 pos(timeline_info_->GetWorldFromTick(time) - kPickingBoxOffset,
                   GetPos()[1] - track_height + 1);
          Vec2 size(kPickingBoxWidth, track_height);
          auto user_data =
              std::make_unique<PickingUserData>(nullptr, [&](PickingId id) -> std::string {
                return GetTracepointTooltip(primitive_assembler, id);
              });
          user_data->custom_data_ = &tracepoint;
          primitive_assembler.AddShadedBox(pos, size, z, white, std::move(user_data));
        });
  }
}

std::string TracepointThreadBar::GetTracepointTooltip(PrimitiveAssembler& primitive_assembler,
                                                      PickingId id) const {
  const auto* user_data = primitive_assembler.GetUserData(id);
  ORBIT_CHECK(user_data && user_data->custom_data_);

  const auto* tracepoint_event_info =
      static_cast<const orbit_client_data::TracepointEventInfo*>(user_data->custom_data_);

  uint64_t tracepoint_id = tracepoint_event_info->tracepoint_id();

  ORBIT_CHECK(capture_data_ != nullptr);

  const orbit_client_data::TracepointInfo* tracepoint_info =
      capture_data_->GetTracepointInfo(tracepoint_id);
  ORBIT_CHECK(tracepoint_info != nullptr);

  if (GetThreadId() == orbit_base::kAllThreadsOfAllProcessesTid) {
    return absl::StrFormat(
        "<b>%s : %s</b><br/>"
        "<i>Tracepoint event</i><br/>"
        "<br/>"
        "<b>Core:</b> %d<br/>"
        "<b>Process:</b> %s [%d]<br/>"
        "<b>Thread:</b> %s [%d]<br/>",
        tracepoint_info->category(), tracepoint_info->name(), tracepoint_event_info->cpu(),
        capture_data_->GetThreadName(tracepoint_event_info->pid()), tracepoint_event_info->pid(),
        capture_data_->GetThreadName(tracepoint_event_info->tid()), tracepoint_event_info->tid());
  } else {
    return absl::StrFormat(
        "<b>%s : %s</b><br/>"
        "<i>Tracepoint event</i><br/>"
        "<br/>"
        "<b>Core:</b> %d<br/>",
        tracepoint_info->category(), tracepoint_info->name(), tracepoint_event_info->cpu());
  }
}

bool TracepointThreadBar::IsEmpty() const {
  return capture_data_ == nullptr ||
         capture_data_->GetNumTracepointsForThreadId(GetThreadId()) == 0;
}

}  // namespace orbit_gl
