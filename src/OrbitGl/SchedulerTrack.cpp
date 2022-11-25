// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SchedulerTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/ThreadColor.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

using orbit_client_protos::TimerInfo;
using orbit_gl::PickingUserData;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

const Color kInactiveColor(100, 100, 100, 255);
const Color kSamePidColor(140, 140, 140, 255);
const Color kSelectionColor(0, 128, 255, 255);

SchedulerTrack::SchedulerTrack(CaptureViewElement* parent,
                               const orbit_gl::TimelineInfoInterface* timeline_info,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                               const orbit_client_data::ModuleManager* module_manager,
                               const orbit_client_data::CaptureData* capture_data,
                               orbit_client_data::TimerData* timer_data)
    : TimerTrack(parent, timeline_info, viewport, layout, app, module_manager, capture_data,
                 timer_data),
      num_cores_{0} {
  SetPinned(false);
}

void SchedulerTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  TimerTrack::OnTimer(timer_info);
  if (num_cores_ <= static_cast<uint32_t>(timer_info.processor())) {
    num_cores_ = timer_info.processor() + 1;
  }
}

float SchedulerTrack::GetHeight() const {
  uint32_t num_gaps = std::max(GetDepth() - 1, 0u);
  return GetHeightAboveTimers() + (GetDepth() * layout_->GetTextCoresHeight()) +
         (num_gaps * layout_->GetSpaceBetweenCores()) + layout_->GetTrackContentBottomMargin();
}

void SchedulerTrack::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                        TextRenderer& /*text_renderer*/, uint64_t min_tick,
                                        uint64_t max_tick, PickingMode /*picking_mode*/) {
  // TODO(b/242971217): The parent class already provides an implementation, but this is completely
  // ignored because we are using an optimized way to render in SchedulerTrack. In the future we
  // will move this implementation to be the defaulted one.
  // TimerTrack::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
  //                                picking_mode);
  ORBIT_SCOPE_WITH_COLOR("SchedulerTrack::DoUpdatePrimitives", kOrbitColorPink);
  visible_timer_count_ = 0;

  const internal::DrawData draw_data =
      GetDrawData(min_tick, max_tick, GetPos()[0], GetWidth(), &primitive_assembler, timeline_info_,
                  viewport_, IsCollapsed(), app_->selected_timer(), app_->GetScopeIdToHighlight(),
                  app_->GetGroupIdToHighlight(), app_->GetHistogramSelectionRange());

  const uint32_t resolution_in_pixels = viewport_->WorldToScreen({GetWidth(), 0})[0];
  const float box_height = GetDefaultBoxHeight();

  for (uint32_t depth = 0; depth < GetDepth(); depth++) {
    const float world_timer_y = GetYFromDepth(depth);
    for (const TimerInfo* timer_info : timer_data_->GetTimersAtDepthDiscretized(
             depth, resolution_in_pixels, min_tick, max_tick)) {
      ++visible_timer_count_;
      const bool is_selected = timer_info == draw_data.selected_timer;

      Color color = GetTimerColor(*timer_info, is_selected, /*is_highlighted=*/false, draw_data);
      std::unique_ptr<PickingUserData> user_data =
          CreatePickingUserData(primitive_assembler, *timer_info);

      auto [box_start_x, box_width] =
          timeline_info_->GetBoxPosXAndWidthFromTicks(timer_info->start(), timer_info->end());
      box_start_x = HorizontalClamp(box_start_x);
      const Vec2 pos = {box_start_x, world_timer_y};
      const Vec2 size = {box_width, box_height};
      primitive_assembler.AddShadedBox(pos, size, draw_data.z, color, std::move(user_data));
    }
  }
}

bool SchedulerTrack::IsTimerActive(const TimerInfo& timer_info) const {
  bool is_same_tid_as_selected = timer_info.thread_id() == app_->selected_thread_id();

  ORBIT_CHECK(capture_data_ != nullptr);
  uint32_t capture_process_id = capture_data_->process_id();
  bool is_same_pid_as_target =
      capture_process_id == 0 || capture_process_id == timer_info.process_id();

  return is_same_tid_as_selected ||
         (app_->selected_thread_id() == orbit_base::kAllProcessThreadsTid && is_same_pid_as_target);
}

Color SchedulerTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected,
                                    bool is_highlighted,
                                    const internal::DrawData& draw_data) const {
  if (is_highlighted) {
    return TimerTrack::kHighlightColor;
  }
  if (is_selected) {
    return kSelectionColor;
  }
  if (!IsTimerActive(timer_info)) {
    const TimerInfo* selected_timer = draw_data.selected_timer;
    bool is_same_pid = selected_timer && timer_info.process_id() == selected_timer->process_id();
    return is_same_pid ? kSamePidColor : kInactiveColor;
  }
  return orbit_gl::GetThreadColor(timer_info.thread_id());
}

float SchedulerTrack::GetYFromDepth(uint32_t depth) const {
  return GetPos()[1] + GetHeightAboveTimers() +
         (layout_->GetTextCoresHeight() * static_cast<float>(depth)) +
         depth * layout_->GetSpaceBetweenCores();
}

std::vector<const orbit_client_protos::TimerInfo*> SchedulerTrack::GetScopesInRange(
    uint64_t start_ns, uint64_t end_ns) const {
  std::vector<const orbit_client_protos::TimerInfo*> result;
  for (const orbit_client_data::TimerChain* chain : timer_data_->GetChains()) {
    for (const auto& block : *chain) {
      if (!block.Intersects(start_ns, end_ns)) continue;
      for (uint64_t i = 0; i < block.size(); ++i) {
        const orbit_client_protos::TimerInfo& timer_info = block[i];
        if (timer_info.start() <= end_ns && timer_info.end() > start_ns) {
          result.push_back(&timer_info);
        }
      }
    }
  }
  return result;
}

std::string SchedulerTrack::GetTooltip() const {
  return "Shows scheduling information for CPU cores";
}

std::string SchedulerTrack::GetBoxTooltip(const PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const {
  const orbit_client_protos::TimerInfo* timer_info = primitive_assembler.GetTimerInfo(id);
  if (!timer_info) {
    return "";
  }

  ORBIT_CHECK(capture_data_ != nullptr);
  return absl::StrFormat(
      "<b>CPU Core activity</b><br/>"
      "<br/>"
      "<b>Core:</b> %d<br/>"
      "<b>Process:</b> %s [%d]<br/>"
      "<b>Thread:</b> %s [%d]<br/>",
      timer_info->processor(), capture_data_->GetThreadName(timer_info->process_id()),
      timer_info->process_id(), capture_data_->GetThreadName(timer_info->thread_id()),
      timer_info->thread_id());
}
