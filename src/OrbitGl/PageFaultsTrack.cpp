// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PageFaultsTrack.h"

#include "ApiUtils/EncodedEvent.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "Geometry.h"
#include "GrpcProtos/Constants.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

namespace orbit_gl {

namespace {
using orbit_capture_client::CaptureEventProcessor;
using orbit_grpc_protos::kMissingInfo;
}  // namespace

PageFaultsTrack::PageFaultsTrack(CaptureViewElement* parent,
                                 const orbit_gl::TimelineInfoInterface* timeline_info,
                                 orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                 const std::string& cgroup_name, uint64_t memory_sampling_period_ms,
                                 const orbit_client_data::CaptureData* capture_data)
    : Track(parent, timeline_info, viewport, layout, capture_data),
      major_page_faults_track_{
          std::make_shared<MajorPageFaultsTrack>(this, timeline_info, viewport, layout, cgroup_name,
                                                 memory_sampling_period_ms, capture_data)},
      minor_page_faults_track_{
          std::make_shared<MinorPageFaultsTrack>(this, timeline_info, viewport, layout, cgroup_name,
                                                 memory_sampling_period_ms, capture_data)} {
  // PageFaults track is collapsed by default. The major and minor page faults subtracks are
  // expanded by default, but not shown while the page faults track is collapsed.
  collapse_toggle_->SetCollapsed(true);
}

std::string PageFaultsTrack::GetLabel() const {
  return collapse_toggle_->IsCollapsed() ? major_page_faults_track_->GetName() : GetName();
}

float PageFaultsTrack::GetHeight() const {
  if (collapse_toggle_->IsCollapsed()) {
    return major_page_faults_track_->GetHeight();
  }

  float height = layout_->GetTrackTabHeight();
  if (major_page_faults_track_->ShouldBeRendered()) {
    height += major_page_faults_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  if (minor_page_faults_track_->ShouldBeRendered()) {
    height += minor_page_faults_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  return height;
}

std::string PageFaultsTrack::GetTooltip() const {
  if (collapse_toggle_->IsCollapsed()) return major_page_faults_track_->GetTooltip();
  return "Shows the minor and major page faults statistics.";
}

std::vector<CaptureViewElement*> PageFaultsTrack::GetAllChildren() const {
  auto result = Track::GetAllChildren();
  result.insert(result.end(), {major_page_faults_track_.get(), minor_page_faults_track_.get()});
  return result;
}

void PageFaultsTrack::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                         uint64_t min_tick, uint64_t max_tick,
                                         PickingMode picking_mode) {
  ORBIT_SCOPE("PageFaultsTrack::DoUpdatePrimitives");
  Track::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);
}

void PageFaultsTrack::UpdatePositionOfSubtracks() {
  const Vec2 pos = GetPos();
  if (collapse_toggle_->IsCollapsed()) {
    major_page_faults_track_->SetPos(pos[0], pos[1]);
    minor_page_faults_track_->SetVisible(false);
    major_page_faults_track_->SetHeadless(true);
    return;
  }

  major_page_faults_track_->SetHeadless(false);
  major_page_faults_track_->SetIndentationLevel(indentation_level_ + 1);
  minor_page_faults_track_->SetVisible(true);
  minor_page_faults_track_->SetIndentationLevel(indentation_level_ + 1);

  float current_y = pos[1] + layout_->GetTrackTabHeight();
  if (major_page_faults_track_->ShouldBeRendered()) {
    current_y += layout_->GetSpaceBetweenSubtracks();
  }
  major_page_faults_track_->SetPos(pos[0], current_y);

  if (minor_page_faults_track_->ShouldBeRendered()) {
    current_y += (layout_->GetSpaceBetweenSubtracks() + major_page_faults_track_->GetHeight());
  }
  minor_page_faults_track_->SetPos(pos[0], current_y);
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
}

uint64_t PageFaultsTrack::GetMinTime() const {
  return std::min(minor_page_faults_track_->GetMinTime(), major_page_faults_track_->GetMinTime());
}

uint64_t PageFaultsTrack::GetMaxTime() const {
  return std::max(minor_page_faults_track_->GetMaxTime(), major_page_faults_track_->GetMaxTime());
}

}  // namespace orbit_gl
