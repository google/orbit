// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PagefaultTrack.h"

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

PagefaultTrack::PagefaultTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                               const std::string& cgroup_name, uint64_t memory_sampling_period_ms,
                               const orbit_client_model::CaptureData* capture_data,
                               uint32_t indentation_level)
    : Track(parent, time_graph, viewport, layout, capture_data, indentation_level),
      major_pagefault_track_{std::make_shared<MajorPagefaultTrack>(
          this, time_graph, viewport, layout, cgroup_name, memory_sampling_period_ms, capture_data,
          indentation_level + 1)},
      minor_pagefault_track_{std::make_shared<MinorPagefaultTrack>(
          this, time_graph, viewport, layout, cgroup_name, memory_sampling_period_ms, capture_data,
          indentation_level + 1)} {
  const std::string kTrackName = "Pagefault Track";
  SetName(kTrackName);
  SetLabel(kTrackName);

  // Pagefault track is collapsed by default. The major and minor pagefault subtracks are expanded
  // by default, but not shown while the pagefault track is collapsed.
  collapse_toggle_->SetCollapsed(true);
}

float PagefaultTrack::GetHeight() const {
  if (collapse_toggle_->IsCollapsed()) {
    return major_pagefault_track_->GetHeight();
  }

  float height = layout_->GetTrackTabHeight();
  if (!major_pagefault_track_->IsEmpty()) {
    height += major_pagefault_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  if (!minor_pagefault_track_->IsEmpty()) {
    height += minor_pagefault_track_->GetHeight() + layout_->GetSpaceBetweenSubtracks();
  }
  return height;
}

std::vector<orbit_gl::CaptureViewElement*> PagefaultTrack::GetVisibleChildren() {
  std::vector<CaptureViewElement*> result;
  if (collapse_toggle_->IsCollapsed()) return result;

  if (!major_pagefault_track_->IsEmpty()) result.push_back(major_pagefault_track_.get());
  if (!minor_pagefault_track_->IsEmpty()) result.push_back(minor_pagefault_track_.get());
  return result;
}

std::string PagefaultTrack::GetTooltip() const {
  if (collapse_toggle_->IsCollapsed()) return major_pagefault_track_->GetTooltip();
  return "Shows the minor and major pagefault statistics.";
}

void PagefaultTrack::Draw(Batcher& batcher, TextRenderer& text_renderer,
                          uint64_t current_mouse_time_ns, PickingMode picking_mode,
                          float z_offset) {
  float track_height = GetHeight();
  float track_width = viewport_->GetVisibleWorldWidth();

  SetPos(viewport_->GetWorldTopLeft()[0], pos_[1]);
  SetSize(track_width, track_height);
  SetLabel(collapse_toggle_->IsCollapsed() ? major_pagefault_track_->GetName() : GetName());

  UpdatePositionOfSubtracks();

  Track::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);

  if (collapse_toggle_->IsCollapsed()) return;

  if (!major_pagefault_track_->IsEmpty()) {
    major_pagefault_track_->SetSize(viewport_->GetVisibleWorldWidth(),
                                    major_pagefault_track_->GetHeight());
    major_pagefault_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode,
                                 z_offset);
  }

  if (!minor_pagefault_track_->IsEmpty()) {
    minor_pagefault_track_->SetSize(viewport_->GetVisibleWorldWidth(),
                                    minor_pagefault_track_->GetHeight());
    minor_pagefault_track_->Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode,
                                 z_offset);
  }
}

void PagefaultTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                      PickingMode picking_mode, float z_offset) {
  UpdatePositionOfSubtracks();

  if (!major_pagefault_track_->IsEmpty()) {
    major_pagefault_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }

  if (collapse_toggle_->IsCollapsed()) return;

  if (!minor_pagefault_track_->IsEmpty()) {
    minor_pagefault_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
}

void PagefaultTrack::UpdatePositionOfSubtracks() {
  if (collapse_toggle_->IsCollapsed()) {
    major_pagefault_track_->SetPos(pos_[0], pos_[1]);
    return;
  }

  float current_y = pos_[1] - layout_->GetTrackTabHeight();
  if (!major_pagefault_track_->IsEmpty()) {
    current_y -= layout_->GetSpaceBetweenSubtracks();
  }
  major_pagefault_track_->SetPos(pos_[0], current_y);

  if (!minor_pagefault_track_->IsEmpty()) {
    current_y -= (layout_->GetSpaceBetweenSubtracks() + major_pagefault_track_->GetHeight());
  }
  minor_pagefault_track_->SetPos(pos_[0], current_y);
}

void PagefaultTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  int64_t system_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kSystemPagefault)));
  int64_t system_major_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kSystemMajorPagefault)));
  int64_t cgroup_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kCGroupPagefault)));
  int64_t cgroup_major_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kCGroupMajorPagefault)));
  int64_t process_minor_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kProcessMinorPagefault)));
  int64_t process_major_pagefaults = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PagefaultEncodingIndex::kProcessMajorPagefault)));

  if (system_major_pagefaults != kMissingInfo && cgroup_major_pagefaults != kMissingInfo &&
      process_major_pagefaults != kMissingInfo) {
    std::array<double, orbit_gl::kBasicPagefaultTrackDimension> values;
    values[static_cast<size_t>(MajorPagefaultTrack::SeriesIndex::kProcess)] =
        static_cast<double>(process_major_pagefaults);
    values[static_cast<size_t>(MajorPagefaultTrack::SeriesIndex::kCGroup)] =
        static_cast<double>(cgroup_major_pagefaults);
    values[static_cast<size_t>(MajorPagefaultTrack::SeriesIndex::kSystem)] =
        static_cast<double>(system_major_pagefaults);
    AddValuesAndUpdateAnnotationsForMajorPagefaultSubtrack(timer_info.start(), values);
  }

  if (system_pagefaults != kMissingInfo && system_major_pagefaults != kMissingInfo &&
      cgroup_pagefaults != kMissingInfo && cgroup_major_pagefaults != kMissingInfo &&
      process_minor_pagefaults != kMissingInfo) {
    std::array<double, orbit_gl::kBasicPagefaultTrackDimension> values;
    values[static_cast<size_t>(MinorPagefaultTrack::SeriesIndex::kProcess)] =
        static_cast<double>(process_minor_pagefaults);
    values[static_cast<size_t>(MinorPagefaultTrack::SeriesIndex::kCGroup)] =
        static_cast<double>(cgroup_pagefaults - cgroup_major_pagefaults);
    values[static_cast<size_t>(MinorPagefaultTrack::SeriesIndex::kSystem)] =
        static_cast<double>(system_pagefaults - system_major_pagefaults);
    AddValuesAndUpdateAnnotationsForMinorPagefaultSubtrack(timer_info.start(), values);
  }

  constexpr uint32_t kDepth = 0;
  std::shared_ptr<orbit_client_data::TimerChain> timer_chain = timers_[kDepth];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<orbit_client_data::TimerChain>();
    timers_[kDepth] = timer_chain;
  }

  timer_chain->emplace_back(timer_info);
}

std::vector<std::shared_ptr<orbit_client_data::TimerChain>> PagefaultTrack::GetAllChains() const {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  for (const auto& pair : timers_) {
    chains.push_back(pair.second);
  }
  return chains;
}

}  // namespace orbit_gl