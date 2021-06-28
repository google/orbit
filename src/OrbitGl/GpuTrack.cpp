// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTrack.h"

#include <absl/time/time.h>

#include <algorithm>
#include <memory>

#include "App.h"
#include "Batcher.h"
#include "ClientModel/CaptureData.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "TriangleToggle.h"
#include "Viewport.h"
#include "absl/strings/str_format.h"

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
                   const orbit_client_model::CaptureData* capture_data, uint32_t indentation_level)
    : Track(parent, time_graph, viewport, layout, capture_data, indentation_level),
      submission_track_{std::make_shared<GpuSubmissionTrack>(this, time_graph, viewport, layout,
                                                             timeline_hash, app, capture_data,
                                                             indentation_level + 1)},
      marker_track_{std::make_shared<GpuDebugMarkerTrack>(this, time_graph, viewport, layout, app,
                                                          capture_data, indentation_level + 1)} {
  timeline_hash_ = timeline_hash;

  std::string timeline =
      app->GetStringManager()->Get(timeline_hash).value_or(std::to_string(timeline_hash));
  std::string label = orbit_gl::MapGpuTimelineToTrackLabel(timeline);
  SetName(timeline);
  SetLabel(label);

  submission_track_->SetName(absl::StrFormat("%s_submissions", timeline));
  marker_track_->SetName(absl::StrFormat("%s_marker", timeline));

  // Gpu are collapsed by default. Their subtracks are expanded by default, but are however not
  // shown while the Gpu track is collapsed.
  collapse_toggle_->SetCollapsed(true);
}

void GpuTrack::OnTimer(const TimerInfo& timer_info) {
  ++num_timers_;
  if (timer_info.start() < min_time_) min_time_ = timer_info.start();
  if (timer_info.end() > max_time_) max_time_ = timer_info.end();

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

std::vector<std::shared_ptr<TimerChain>> GpuTrack::GetAllChains() const {
  std::vector<std::shared_ptr<TimerChain>> all_chains = submission_track_->GetAllChains();
  std::vector<std::shared_ptr<TimerChain>> marker_chains = marker_track_->GetAllChains();
  all_chains.insert(all_chains.begin(), marker_chains.begin(), marker_chains.end());
  return all_chains;
}

std::vector<std::shared_ptr<TimerChain>> GpuTrack::GetAllSerializableChains() const {
  std::vector<std::shared_ptr<TimerChain>> all_chains =
      submission_track_->GetAllSerializableChains();
  std::vector<std::shared_ptr<TimerChain>> marker_chains =
      marker_track_->GetAllSerializableChains();
  all_chains.insert(all_chains.begin(), marker_chains.begin(), marker_chains.end());
  return all_chains;
}

void GpuTrack::UpdatePositionOfSubtracks() {
  if (collapse_toggle_->IsCollapsed()) {
    submission_track_->SetPos(pos_[0], pos_[1]);
    return;
  }
  float current_y = pos_[1] - layout_->GetTrackTabHeight();
  if (!submission_track_->IsEmpty()) {
    current_y -= layout_->GetSpaceBetweenGpuSubtracks();
  }
  submission_track_->SetPos(pos_[0], current_y);
  if (!marker_track_->IsEmpty()) {
    current_y -= (layout_->GetSpaceBetweenGpuSubtracks() + submission_track_->GetHeight());
  }

  marker_track_->SetPos(pos_[0], current_y);
}

float GpuTrack::GetHeight() const {
  if (collapse_toggle_->IsCollapsed()) {
    return submission_track_->GetHeight();
  }
  float height = layout_->GetTrackTabHeight();
  if (!submission_track_->IsEmpty()) {
    height += submission_track_->GetHeight();
  }
  if (!marker_track_->IsEmpty()) {
    height += marker_track_->GetHeight();
  }
  if (!submission_track_->IsEmpty() && !marker_track_->IsEmpty()) {
    height += layout_->GetSpaceBetweenGpuSubtracks();
  }
  if (!(submission_track_->IsEmpty() && marker_track_->IsEmpty())) {
    height += layout_->GetSpaceBetweenGpuSubtracks();
  }
  return height;
}

void GpuTrack::Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
                    PickingMode picking_mode, float z_offset) {
  float track_height = GetHeight();
  float track_width = viewport_->GetVisibleWorldWidth();

  SetPos(viewport_->GetWorldTopLeft()[0], pos_[1]);
  SetSize(track_width, track_height);

  UpdatePositionOfSubtracks();

  Track::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);

  if (collapse_toggle_->IsCollapsed()) {
    return;
  }

  if (!submission_track_->IsEmpty()) {
    submission_track_->SetSize(viewport_->GetVisibleWorldWidth(), submission_track_->GetHeight());
    submission_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);
  }

  if (!marker_track_->IsEmpty()) {
    marker_track_->SetSize(viewport_->GetVisibleWorldWidth(), marker_track_->GetHeight());
    marker_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);
  }
}

void GpuTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                PickingMode picking_mode, float z_offset) {
  UpdatePositionOfSubtracks();

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

const TextBox* GpuTrack::GetLeft(const TextBox* textbox) const {
  const TimerInfo& timer_info = textbox->GetTimerInfo();
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetLeft(textbox);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetLeft(textbox);
    default:
      UNREACHABLE();
  }
}

const TextBox* GpuTrack::GetRight(const TextBox* textbox) const {
  const TimerInfo& timer_info = textbox->GetTimerInfo();
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetRight(textbox);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetRight(textbox);
    default:
      UNREACHABLE();
  }
}

const TextBox* GpuTrack::GetUp(const TextBox* textbox) const {
  const TimerInfo& timer_info = textbox->GetTimerInfo();
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetUp(textbox);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetUp(textbox);
    default:
      UNREACHABLE();
  }
}

const TextBox* GpuTrack::GetDown(const TextBox* textbox) const {
  const TimerInfo& timer_info = textbox->GetTimerInfo();
  switch (timer_info.type()) {
    case TimerInfo::kGpuActivity:
      [[fallthrough]];
    case TimerInfo::kGpuCommandBuffer:
      return submission_track_->GetDown(textbox);
    case TimerInfo::kGpuDebugMarker:
      return marker_track_->GetDown(textbox);
    default:
      UNREACHABLE();
  }
}
