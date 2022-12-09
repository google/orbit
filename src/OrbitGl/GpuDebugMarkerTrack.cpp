// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GpuDebugMarkerTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <memory>
#include <optional>

#include "ClientProtos/capture_data.pb.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimeGraphLayout.h"

using orbit_client_protos::TimerInfo;
using orbit_gl::PrimitiveAssembler;

GpuDebugMarkerTrack::GpuDebugMarkerTrack(CaptureViewElement* parent,
                                         const orbit_gl::TimelineInfoInterface* timeline_info,
                                         orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                         uint64_t timeline_hash, OrbitApp* app,
                                         const orbit_client_data::ModuleManager* module_manager,
                                         const orbit_client_data::CaptureData* capture_data,
                                         orbit_client_data::TimerData* timer_data)
    : TimerTrack(parent, timeline_info, viewport, layout, app, module_manager, capture_data,
                 timer_data),
      string_manager_{app->GetStringManager()},
      timeline_hash_{timeline_hash} {}

std::string GpuDebugMarkerTrack::GetName() const {
  return absl::StrFormat(
      "%s_marker", string_manager_->Get(timeline_hash_).value_or(std::to_string(timeline_hash_)));
}

std::string GpuDebugMarkerTrack::GetTooltip() const {
  return "Shows execution times for Vulkan debug markers";
}

Color GpuDebugMarkerTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected,
                                         bool is_highlighted,
                                         const internal::DrawData& /*draw_data*/) const {
  ORBIT_CHECK(timer_info.type() == TimerInfo::kGpuDebugMarker);
  const Color inactive_color(100, 100, 100, 255);
  const Color selection_color(0, 128, 255, 255);
  if (is_highlighted) {
    return TimerTrack::kHighlightColor;
  }
  if (is_selected) {
    return selection_color;
  }
  if (!IsTimerActive(timer_info)) {
    return inactive_color;
  }
  if (timer_info.has_color()) {
    ORBIT_CHECK(timer_info.color().red() < 256);
    ORBIT_CHECK(timer_info.color().green() < 256);
    ORBIT_CHECK(timer_info.color().blue() < 256);
    ORBIT_CHECK(timer_info.color().alpha() < 256);
    return {static_cast<uint8_t>(timer_info.color().red()),
            static_cast<uint8_t>(timer_info.color().green()),
            static_cast<uint8_t>(timer_info.color().blue()),
            static_cast<uint8_t>(timer_info.color().alpha())};
  }
  std::string marker_text = string_manager_->Get(timer_info.user_data_key()).value_or("");
  return TimeGraph::GetColor(marker_text);
}

std::string GpuDebugMarkerTrack::GetTimesliceText(const TimerInfo& timer_info) const {
  ORBIT_CHECK(timer_info.type() == TimerInfo::kGpuDebugMarker);

  std::string time = GetDisplayTime(timer_info);
  return absl::StrFormat("%s  %s", string_manager_->Get(timer_info.user_data_key()).value_or(""),
                         time);
}

std::string GpuDebugMarkerTrack::GetBoxTooltip(const PrimitiveAssembler& primitive_assembler,
                                               PickingId id) const {
  const TimerInfo* timer_info = primitive_assembler.GetTimerInfo(id);
  if (timer_info == nullptr) {
    return "";
  }

  ORBIT_CHECK(timer_info->type() == TimerInfo::kGpuDebugMarker);

  std::string marker_text = string_manager_->Get(timer_info->user_data_key()).value_or("");
  return absl::StrFormat(
      "<b>Vulkan Debug Marker</b><br/>"
      "<i>At the marker's begin and end `vkCmdWriteTimestamp`s have been "
      "inserted. The GPU timestamps get aligned with the corresponding hardware execution of the "
      "submission.</i>"
      "<br/>"
      "<br/>"
      "<b>Marker text:</b> %s<br/>"
      "<b>Submitted from process:</b> %s [%d]<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      marker_text, capture_data_->GetThreadName(timer_info->process_id()), timer_info->process_id(),
      capture_data_->GetThreadName(timer_info->thread_id()), timer_info->thread_id(),
      orbit_display_formats::GetDisplayTime(TicksToDuration(timer_info->start(), timer_info->end()))
          .c_str());
}

float GpuDebugMarkerTrack::GetYFromDepth(uint32_t depth) const {
  if (IsCollapsed()) {
    depth = 0;
  }
  return GetPos()[1] + layout_->GetTrackTabHeight() + layout_->GetTrackContentTopMargin() +
         layout_->GetTextBoxHeight() * depth;
}

float GpuDebugMarkerTrack::GetHeight() const {
  bool collapsed = IsCollapsed();
  uint32_t depth = collapsed ? std::min<uint32_t>(1, GetDepth()) : GetDepth();
  return layout_->GetTrackTabHeight() + layout_->GetTrackContentTopMargin() +
         layout_->GetTextBoxHeight() * depth + layout_->GetTrackContentBottomMargin();
}

bool GpuDebugMarkerTrack::TimerFilter(const TimerInfo& timer_info) const {
  if (IsCollapsed()) {
    return timer_info.depth() == 0;
  }
  return true;
}
