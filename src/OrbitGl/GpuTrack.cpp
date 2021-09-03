// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTrack.h"

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <algorithm>
#include <memory>

#include "App.h"
#include "Batcher.h"
#include "ClientData/CaptureData.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TriangleToggle.h"
#include "Viewport.h"
#include "capture_data.pb.h"

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

GpuTrack::GpuTrack(CaptureViewElement* parent, TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                   TimeGraphLayout* layout, uint64_t timeline_hash, OrbitApp* app,
                   const orbit_client_data::CaptureData* capture_data,
                   orbit_client_data::TrackPaneData* submission_track_data,
                   orbit_client_data::TrackPaneData* marker_track_data)
    : Track(parent, time_graph, viewport, layout, capture_data),
      string_manager_{app->GetStringManager()},
      submission_track_{std::make_shared<GpuSubmissionTrack>(this, time_graph, viewport, layout,
                                                             timeline_hash, app, capture_data,
                                                             submission_track_data)},
      marker_track_{std::make_shared<GpuDebugMarkerTrack>(
          this, time_graph, viewport, layout, timeline_hash, app, capture_data, marker_track_data)},
      timeline_hash_{timeline_hash} {
  // Gpu are collapsed by default. Their subtracks are expanded by default, but are however not
  // shown while the Gpu track is collapsed.
  collapse_toggle_->SetCollapsed(true);
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
      UNREACHABLE();
  }
}

void GpuTrack::UpdatePositionOfSubtracks() {
  const Vec2 pos = GetPos();
  if (collapse_toggle_->IsCollapsed()) {
    submission_track_->SetPos(pos[0], pos[1]);
    return;
  }
  float current_y = pos[1] + layout_->GetTrackTabHeight();
  if (!submission_track_->IsEmpty()) {
    current_y += layout_->GetSpaceBetweenSubtracks();
  }
  submission_track_->SetPos(pos[0], current_y);
  if (!marker_track_->IsEmpty()) {
    current_y += (layout_->GetSpaceBetweenSubtracks() + submission_track_->GetHeight());
  }

  marker_track_->SetPos(pos[0], current_y);
}

float GpuTrack::GetHeight() const {
  if (collapse_toggle_->IsCollapsed()) {
    return submission_track_->GetHeight();
  }
  float height = layout_->GetTrackTabHeight();
  if (!submission_track_->IsEmpty()) {
    height += submission_track_->GetHeight();
    height += layout_->GetSpaceBetweenSubtracks();
  }
  if (!marker_track_->IsEmpty()) {
    height += marker_track_->GetHeight();
    height += layout_->GetSpaceBetweenSubtracks();
  }
  return height;
}

// TODO(b/176216022): Make a general interface for capture view elements for setting the width to
// every child.
void GpuTrack::SetWidth(float width) {
  Track::SetWidth(width);
  submission_track_->SetWidth(width);
  marker_track_->SetWidth(width);
}

void GpuTrack::Draw(Batcher& batcher, TextRenderer& text_renderer,
                    const DrawContext& draw_context) {
  Track::Draw(batcher, text_renderer, draw_context);

  if (collapse_toggle_->IsCollapsed()) {
    return;
  }
  const DrawContext sub_track_draw_context = draw_context.IncreasedIndentationLevel();
  if (!submission_track_->IsEmpty()) {
    submission_track_->Draw(batcher, text_renderer, sub_track_draw_context);
  }

  if (!marker_track_->IsEmpty()) {
    marker_track_->Draw(batcher, text_renderer, sub_track_draw_context);
  }
}

void GpuTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                PickingMode picking_mode, float z_offset) {
  const bool is_collapsed = collapse_toggle_->IsCollapsed();

  if (!submission_track_->IsEmpty()) {
    submission_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }

  if (is_collapsed) {
    return;
  }

  if (!marker_track_->IsEmpty()) {
    marker_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
}

std::vector<orbit_gl::CaptureViewElement*> GpuTrack::GetVisibleChildren() {
  std::vector<CaptureViewElement*> result;

  if (collapse_toggle_->IsCollapsed()) {
    return result;
  }

  if (!submission_track_->IsEmpty()) {
    result.push_back(submission_track_.get());
  }

  if (!marker_track_->IsEmpty()) {
    result.push_back(marker_track_.get());
  }
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
      UNREACHABLE();
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
      UNREACHABLE();
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
      UNREACHABLE();
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
      UNREACHABLE();
  }
}
