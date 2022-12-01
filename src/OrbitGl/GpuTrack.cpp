// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GpuTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <memory>

#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"
#include "StringManager/StringManager.h"

using orbit_client_protos::TimerInfo;

namespace orbit_gl {

std::string MapGpuTimelineToTrackLabel(std::string_view timeline) {
  std::string queue_pretty_name;
  std::string timeline_label(timeline);
  if (timeline.rfind("gfx", 0) == 0) {
    queue_pretty_name = "Graphics queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("sdma", 0) == 0) {
    queue_pretty_name = "DMA queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("comp", 0) == 0) {
    queue_pretty_name = "Compute queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("vce", 0) == 0) {
    queue_pretty_name = "Video Coding Engine";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  return absl::StrFormat("%s%s", queue_pretty_name, timeline_label);
}

}  // namespace orbit_gl

GpuTrack::GpuTrack(CaptureViewElement* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
                   orbit_gl::Viewport* viewport, TimeGraphLayout* layout, uint64_t timeline_hash,
                   OrbitApp* app, const orbit_client_data::ModuleManager* module_manager,
                   const orbit_client_data::CaptureData* capture_data,
                   orbit_client_data::TimerData* submission_timer_data,
                   orbit_client_data::TimerData* marker_timer_data,
                   orbit_string_manager::StringManager* string_manager)
    : Track(parent, timeline_info, viewport, layout, module_manager, capture_data),
      string_manager_{string_manager},
      submission_track_{std::make_shared<GpuSubmissionTrack>(this, timeline_info, viewport, layout,
                                                             timeline_hash, app, module_manager,
                                                             capture_data, submission_timer_data)},
      marker_track_{std::make_shared<GpuDebugMarkerTrack>(this, timeline_info, viewport, layout,
                                                          timeline_hash, app, module_manager,
                                                          capture_data, marker_timer_data)},
      timeline_hash_{timeline_hash} {
  // Gpu are collapsed by default. Their subtracks are expanded by default, but are however not
  // shown while the Gpu track is collapsed.
  SetCollapsed(true);
}

void GpuTrack::OnTimer(const TimerInfo& timer_info) {
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      submission_track_->OnTimer(timer_info);
      break;
    case TimerInfo::kGpuDebugMarker:
      marker_track_->OnTimer(timer_info);
      break;
    default:
      ORBIT_UNREACHABLE();
  }
}

void GpuTrack::UpdatePositionOfSubtracks() {
  const Vec2 pos = GetPos();
  if (IsCollapsed()) {
    submission_track_->SetPos(pos[0], pos[1]);
    marker_track_->SetVisible(false);
    submission_track_->SetHeadless(true);
    return;
  }
  marker_track_->SetVisible(true);
  marker_track_->SetIndentationLevel(indentation_level_ + 1);
  submission_track_->SetHeadless(false);
  submission_track_->SetIndentationLevel(indentation_level_ + 1);

  float current_y = pos[1] + layout_->GetTrackTabHeight();
  if (submission_track_->ShouldBeRendered()) {
    current_y += layout_->GetSpaceBetweenSubtracks();
  }
  submission_track_->SetPos(pos[0], current_y);
  if (marker_track_->ShouldBeRendered()) {
    current_y += (layout_->GetSpaceBetweenSubtracks() + submission_track_->GetHeight());
  }

  marker_track_->SetPos(pos[0], current_y);
}

float GpuTrack::GetHeight() const {
  if (IsCollapsed()) {
    return submission_track_->GetHeight();
  }
  float height = layout_->GetTrackTabHeight();
  if (submission_track_->ShouldBeRendered()) {
    height += submission_track_->GetHeight();
    height += layout_->GetSpaceBetweenSubtracks();
  }
  if (marker_track_->ShouldBeRendered()) {
    height += marker_track_->GetHeight();
    height += layout_->GetSpaceBetweenSubtracks();
  }
  return height;
}

std::vector<orbit_gl::CaptureViewElement*> GpuTrack::GetAllChildren() const {
  auto result = Track::GetAllChildren();
  result.insert(result.end(), {submission_track_.get(), marker_track_.get()});
  return result;
}

std::string GpuTrack::GetTooltip() const {
  return "Shows scheduling and execution times for selected GPU job "
         "submissions and debug markers";
}

const TimerInfo* GpuTrack::GetLeft(const TimerInfo& timer_info) const {
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetLeft(timer_info);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetLeft(timer_info);
    default:
      ORBIT_UNREACHABLE();
  }
}

const TimerInfo* GpuTrack::GetRight(const TimerInfo& timer_info) const {
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetRight(timer_info);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetRight(timer_info);
    default:
      ORBIT_UNREACHABLE();
  }
}

const TimerInfo* GpuTrack::GetUp(const TimerInfo& timer_info) const {
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetUp(timer_info);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetUp(timer_info);
    default:
      ORBIT_UNREACHABLE();
  }
}

const TimerInfo* GpuTrack::GetDown(const TimerInfo& timer_info) const {
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetDown(timer_info);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetDown(timer_info);
    default:
      ORBIT_UNREACHABLE();
  }
}
