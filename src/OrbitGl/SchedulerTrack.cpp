// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SchedulerTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include "App.h"
#include "Batcher.h"
#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

using orbit_client_protos::TimerInfo;

const Color kInactiveColor(100, 100, 100, 255);
const Color kSamePidColor(140, 140, 140, 255);
const Color kSelectionColor(0, 128, 255, 255);

SchedulerTrack::SchedulerTrack(CaptureViewElement* parent,
                               const orbit_gl::TimelineInfoInterface* timeline_info,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                               const orbit_client_data::CaptureData* capture_data,
                               orbit_client_data::TimerData* timer_data)
    : TimerTrack(parent, timeline_info, viewport, layout, app, capture_data, timer_data),
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
  return GetHeaderHeight() + (GetDepth() * layout_->GetTextCoresHeight()) +
         (num_gaps * layout_->GetSpaceBetweenCores()) + layout_->GetTrackContentBottomMargin();
}

void SchedulerTrack::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                        uint64_t min_tick, uint64_t max_tick,
                                        PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("SchedulerTrack::DoUpdatePrimitives", kOrbitColorPink);
  TimerTrack::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);
}

bool SchedulerTrack::IsTimerActive(const TimerInfo& timer_info) const {
  bool is_same_tid_as_selected = timer_info.thread_id() == app_->selected_thread_id();

  CHECK(capture_data_ != nullptr);
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
  return TimeGraph::GetThreadColor(timer_info.thread_id());
}

float SchedulerTrack::GetYFromTimer(const TimerInfo& timer_info) const {
  uint32_t num_gaps = timer_info.depth();
  return GetPos()[1] + GetHeaderHeight() +
         (layout_->GetTextCoresHeight() * static_cast<float>(timer_info.depth())) +
         num_gaps * layout_->GetSpaceBetweenCores();
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

std::string SchedulerTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const orbit_client_protos::TimerInfo* timer_info = batcher.GetTimerInfo(id);
  if (!timer_info) {
    return "";
  }

  CHECK(capture_data_ != nullptr);
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
