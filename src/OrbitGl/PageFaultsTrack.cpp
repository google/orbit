// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PageFaultsTrack.h"

#include "Api/EncodedEvent.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "Geometry.h"
#include "GrpcProtos/Constants.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

namespace orbit_gl {

namespace {
using orbit_capture_client::CaptureEventProcessor;
using orbit_grpc_protos::kMissingInfo;
}  // namespace

PageFaultsTrack::PageFaultsTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                                 orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                 const std::string& cgroup_name, uint64_t memory_sampling_period_ms,
                                 const orbit_client_model::CaptureData* capture_data,
                                 uint32_t indentation_level)
    : Track(parent, time_graph, viewport, layout, capture_data, indentation_level),
      major_page_faults_track_{std::make_shared<MajorPageFaultsTrack>(
          this, time_graph, viewport, layout, cgroup_name, memory_sampling_period_ms, capture_data,
          indentation_level + 1)},
      minor_page_faults_track_{std::make_shared<MinorPageFaultsTrack>(
          this, time_graph, viewport, layout, cgroup_name, memory_sampling_period_ms, capture_data,
          indentation_level + 1)} {
  const std::string kTrackName = "Page Faults";
  SetName(kTrackName);
  SetLabel(kTrackName);

  // PageFaults track is collapsed by default. The major and minor page faults subtracks are
  // expanded by default, but not shown while the page faults track is collapsed.
  collapse_toggle_->SetCollapsed(true);
}

float PageFaultsTrack::GetHeight() const {
  if (collapse_toggle_->IsCollapsed()) {
    return major_page_faults_track_->GetHeight();
  }

  float height = layout_->GetTrackTabHeight();
  if (!major_page_faults_track_->IsEmpty()) {
    height += major_page_faults_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  if (!minor_page_faults_track_->IsEmpty()) {
    height += minor_page_faults_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  return height;
}

std::vector<orbit_gl::CaptureViewElement*> PageFaultsTrack::GetVisibleChildren() {
  std::vector<CaptureViewElement*> result;
  if (collapse_toggle_->IsCollapsed()) return result;

  if (!major_page_faults_track_->IsEmpty()) result.push_back(major_page_faults_track_.get());
  if (!minor_page_faults_track_->IsEmpty()) result.push_back(minor_page_faults_track_.get());
  return result;
}

std::string PageFaultsTrack::GetTooltip() const {
  if (collapse_toggle_->IsCollapsed()) return major_page_faults_track_->GetTooltip();
  return "Shows the minor and major page faults statistics.";
}

void PageFaultsTrack::Draw(Batcher& batcher, TextRenderer& text_renderer,
                           uint64_t current_mouse_time_ns, PickingMode picking_mode,
                           float z_offset) {
  float track_height = GetHeight();
  float track_width = viewport_->GetVisibleWorldWidth();

  SetPos(viewport_->GetWorldTopLeft()[0], pos_[1]);
  SetSize(track_width, track_height);
  SetLabel(collapse_toggle_->IsCollapsed() ? major_page_faults_track_->GetName() : GetName());

  UpdatePositionOfSubtracks();

  Track::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);

  if (collapse_toggle_->IsCollapsed()) return;

  if (!major_page_faults_track_->IsEmpty()) {
    major_page_faults_track_->SetSize(viewport_->GetVisibleWorldWidth(),
                                      major_page_faults_track_->GetHeight());
    major_page_faults_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode,
                                   z_offset);
  }

  if (!minor_page_faults_track_->IsEmpty()) {
    minor_page_faults_track_->SetSize(viewport_->GetVisibleWorldWidth(),
                                      minor_page_faults_track_->GetHeight());
    minor_page_faults_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode,
                                   z_offset);
  }
}

void PageFaultsTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                       PickingMode picking_mode, float z_offset) {
  UpdatePositionOfSubtracks();

  if (!major_page_faults_track_->IsEmpty()) {
    major_page_faults_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }

  if (collapse_toggle_->IsCollapsed()) return;

  if (!minor_page_faults_track_->IsEmpty()) {
    minor_page_faults_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
}

void PageFaultsTrack::UpdatePositionOfSubtracks() {
  if (collapse_toggle_->IsCollapsed()) {
    major_page_faults_track_->SetPos(pos_[0], pos_[1]);
    return;
  }

  float current_y = pos_[1] - layout_->GetTrackTabHeight();
  if (!major_page_faults_track_->IsEmpty()) {
    current_y -= layout_->GetSpaceBetweenSubtracks();
  }
  major_page_faults_track_->SetPos(pos_[0], current_y);

  if (!minor_page_faults_track_->IsEmpty()) {
    current_y -= (layout_->GetSpaceBetweenSubtracks() + major_page_faults_track_->GetHeight());
  }
  minor_page_faults_track_->SetPos(pos_[0], current_y);
}

void PageFaultsTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  int64_t system_page_faults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PageFaultsEncodingIndex::kSystemPageFaults)));
  int64_t system_major_page_faults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PageFaultsEncodingIndex::kSystemMajorPageFaults)));
  int64_t cgroup_page_faults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PageFaultsEncodingIndex::kCGroupPageFaults)));
  int64_t cgroup_major_page_faults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PageFaultsEncodingIndex::kCGroupMajorPageFaults)));
  int64_t process_minor_page_faults =
      orbit_api::Decode<int64_t>(timer_info.registers(static_cast<size_t>(
          CaptureEventProcessor::PageFaultsEncodingIndex::kProcessMinorPageFaults)));
  int64_t process_major_page_faults =
      orbit_api::Decode<int64_t>(timer_info.registers(static_cast<size_t>(
          CaptureEventProcessor::PageFaultsEncodingIndex::kProcessMajorPageFaults)));

  if (system_major_page_faults != kMissingInfo && cgroup_major_page_faults != kMissingInfo &&
      process_major_page_faults != kMissingInfo) {
    std::array<double, orbit_gl::kBasicPageFaultsTrackDimension> values;
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kProcess)] =
        static_cast<double>(process_major_page_faults);
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kCGroup)] =
        static_cast<double>(cgroup_major_page_faults);
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kSystem)] =
        static_cast<double>(system_major_page_faults);
    AddValuesAndUpdateAnnotationsForMajorPageFaultsSubtrack(timer_info.start(), values);
  }

  if (system_page_faults != kMissingInfo && system_major_page_faults != kMissingInfo &&
      cgroup_page_faults != kMissingInfo && cgroup_major_page_faults != kMissingInfo &&
      process_minor_page_faults != kMissingInfo) {
    std::array<double, orbit_gl::kBasicPageFaultsTrackDimension> values;
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kProcess)] =
        static_cast<double>(process_minor_page_faults);
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kCGroup)] =
        static_cast<double>(cgroup_page_faults - cgroup_major_page_faults);
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kSystem)] =
        static_cast<double>(system_page_faults - system_major_page_faults);
    AddValuesAndUpdateAnnotationsForMinorPageFaultsSubtrack(timer_info.start(), values);
  }

  track_data_->AddTimer(/*depth=*/0, timer_info);
}

}  // namespace orbit_gl