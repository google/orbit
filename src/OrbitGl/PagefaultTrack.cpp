// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PagefaultTrack.h"

#include "Geometry.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

namespace orbit_gl {

PagefaultTrack::PagefaultTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                               std::array<std::string, kBasicPagefaultTrackDimension> series_names,
                               const orbit_client_model::CaptureData* capture_data,
                               uint32_t indentation_level)
    : Track(parent, time_graph, viewport, layout, capture_data, indentation_level),
      major_pagefault_track_{std::make_shared<BasicPagefaultTrack>(
          this, time_graph, viewport, layout, "Major Pagefault Track", series_names, capture_data,
          indentation_level + 1)},
      minor_pagefault_track_{std::make_shared<BasicPagefaultTrack>(
          this, time_graph, viewport, layout, "Minor Pagefault Track", series_names, capture_data,
          indentation_level + 1)} {
  SetName("Pagefault Track");
  SetLabel("Pagefault Track");

  const std::string kMajorPagefaultSubtrackTabTooltip =
      "Shows major pagefault statistics. A major pagefault occurs when the requested page does "
      "not reside in the main memory or CPU cache, and has to be swapped from an external "
      "storage. The major pagefaults might cause slow performance of the target process.";
  major_pagefault_track_->SetTrackTabTooltip(kMajorPagefaultSubtrackTabTooltip);

  const std::string kMinorPagefaultSubtrackTabTooltip =
      "Shows minor pagefault statistics. A minor pagefault occurs when the requested page "
      "resides in main memory but the process cannot access it.";
  minor_pagefault_track_->SetTrackTabTooltip(kMinorPagefaultSubtrackTabTooltip);
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
  SetLabel(collapse_toggle_->IsCollapsed() ? "Major Pagefault Track" : "Pagefault Track");

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
  constexpr uint32_t kDepth = 0;
  std::shared_ptr<TimerChain> timer_chain = timers_[kDepth];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<TimerChain>();
    timers_[kDepth] = timer_chain;
  }

  timer_chain->emplace_back(timer_info);
}

std::vector<std::shared_ptr<TimerChain>> PagefaultTrack::GetAllChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& pair : timers_) {
    chains.push_back(pair.second);
  }
  return chains;
}

}  // namespace orbit_gl