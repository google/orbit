// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/PageFaultsTrack.h"

#include <GteVector.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

PageFaultsTrack::PageFaultsTrack(CaptureViewElement* parent,
                                 const orbit_gl::TimelineInfoInterface* timeline_info,
                                 orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                 std::string cgroup_name, uint64_t memory_sampling_period_ms,
                                 const orbit_client_data::ModuleManager* module_manager,
                                 const orbit_client_data::CaptureData* capture_data)
    : Track(parent, timeline_info, viewport, layout, module_manager, capture_data),
      major_page_faults_track_{std::make_shared<MajorPageFaultsTrack>(
          this, timeline_info, viewport, layout, cgroup_name, memory_sampling_period_ms,
          module_manager, capture_data)},
      minor_page_faults_track_{std::make_shared<MinorPageFaultsTrack>(
          this, timeline_info, viewport, layout, std::move(cgroup_name), memory_sampling_period_ms,
          module_manager, capture_data)} {
  // PageFaults track is collapsed by default. The major and minor page faults subtracks are
  // expanded by default, but not shown while the page faults track is collapsed.
  SetCollapsed(true);
}

std::string PageFaultsTrack::GetLabel() const {
  return IsCollapsed() ? major_page_faults_track_->GetName() : GetName();
}

float PageFaultsTrack::GetHeight() const {
  if (IsCollapsed()) {
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
  if (IsCollapsed()) return major_page_faults_track_->GetTooltip();
  return "Shows the minor and major page faults statistics.";
}

std::vector<CaptureViewElement*> PageFaultsTrack::GetAllChildren() const {
  auto result = Track::GetAllChildren();
  result.insert(result.end(), {major_page_faults_track_.get(), minor_page_faults_track_.get()});
  return result;
}

void PageFaultsTrack::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                         TextRenderer& text_renderer, uint64_t min_tick,
                                         uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE("PageFaultsTrack::DoUpdatePrimitives");
  Track::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick, picking_mode);
}

void PageFaultsTrack::UpdatePositionOfSubtracks() {
  const Vec2 pos = GetPos();
  if (IsCollapsed()) {
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

void PageFaultsTrack::OnPageFaultsInfo(const orbit_client_data::PageFaultsInfo& page_faults_info) {
  if (page_faults_info.HasMajorPageFaultsInfo()) {
    std::array<double, orbit_gl::kBasicPageFaultsTrackDimension> values{};
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kProcess)] =
        static_cast<double>(page_faults_info.process_major_page_faults);
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kCGroup)] =
        static_cast<double>(page_faults_info.cgroup_major_page_faults);
    values[static_cast<size_t>(MajorPageFaultsTrack::SeriesIndex::kSystem)] =
        static_cast<double>(page_faults_info.system_major_page_faults);
    AddValuesAndUpdateAnnotationsForMajorPageFaultsSubtrack(page_faults_info.timestamp_ns, values);
  }

  if (page_faults_info.HasMinorPageFaultsInfo()) {
    std::array<double, orbit_gl::kBasicPageFaultsTrackDimension> values{};
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kProcess)] =
        static_cast<double>(page_faults_info.process_minor_page_faults);
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kCGroup)] = static_cast<double>(
        page_faults_info.cgroup_page_faults - page_faults_info.cgroup_major_page_faults);
    values[static_cast<size_t>(MinorPageFaultsTrack::SeriesIndex::kSystem)] = static_cast<double>(
        page_faults_info.system_page_faults - page_faults_info.system_major_page_faults);
    AddValuesAndUpdateAnnotationsForMinorPageFaultsSubtrack(page_faults_info.timestamp_ns, values);
  }
}

uint64_t PageFaultsTrack::GetMinTime() const {
  return std::min(minor_page_faults_track_->GetMinTime(), major_page_faults_track_->GetMinTime());
}

uint64_t PageFaultsTrack::GetMaxTime() const {
  return std::max(minor_page_faults_track_->GetMaxTime(), major_page_faults_track_->GetMaxTime());
}

}  // namespace orbit_gl
