// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuDebugMarkerTrack.h"

#include <absl/time/time.h>

#include <algorithm>
#include <memory>

#include "App.h"
#include "Batcher.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "TriangleToggle.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::TimerInfo;

GpuDebugMarkerTrack::GpuDebugMarkerTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                                         orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                         OrbitApp* app,
                                         const orbit_client_model::CaptureData* capture_data,
                                         uint32_t indentation_level)
    : TimerTrack(parent, time_graph, viewport, layout, app, capture_data, indentation_level) {
  SetLabel("Debug Markers");
  draw_background_ = false;
  string_manager_ = app->GetStringManager();
}

std::string GpuDebugMarkerTrack::GetTooltip() const {
  return "Shows execution times for Vulkan debug markers";
}

Color GpuDebugMarkerTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected,
                                         bool is_highlighted) const {
  CHECK(timer_info.type() == TimerInfo::kGpuDebugMarker);
  const Color kInactiveColor(100, 100, 100, 255);
  const Color kSelectionColor(0, 128, 255, 255);
  if (is_highlighted) {
    return TimerTrack::kHighlightColor;
  }
  if (is_selected) {
    return kSelectionColor;
  }
  if (!IsTimerActive(timer_info)) {
    return kInactiveColor;
  }
  if (timer_info.has_color()) {
    CHECK(timer_info.color().red() < 256);
    CHECK(timer_info.color().green() < 256);
    CHECK(timer_info.color().blue() < 256);
    CHECK(timer_info.color().alpha() < 256);
    return Color(static_cast<uint8_t>(timer_info.color().red()),
                 static_cast<uint8_t>(timer_info.color().green()),
                 static_cast<uint8_t>(timer_info.color().blue()),
                 static_cast<uint8_t>(timer_info.color().alpha()));
  }
  std::string marker_text = string_manager_->Get(timer_info.user_data_key()).value_or("");
  return TimeGraph::GetColor(marker_text);
}

void GpuDebugMarkerTrack::SetTimesliceText(const TimerInfo& timer_info, float min_x, float z_offset,
                                           TextBox* text_box) {
  CHECK(timer_info.type() == TimerInfo::kGpuDebugMarker);

  if (text_box->GetText().empty()) {
    std::string time = orbit_display_formats::GetDisplayTime(
        absl::Nanoseconds(timer_info.end() - timer_info.start()));
    text_box->SetElapsedTimeTextLength(time.length());
    std::string text = absl::StrFormat(
        "%s  %s", string_manager_->Get(timer_info.user_data_key()).value_or(""), time.c_str());
    text_box->SetText(text);
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout_->GetTextOffset(),
      GlCanvas::kZValueBox + z_offset, kTextWhite, text_box->GetElapsedTimeTextLength(),
      layout_->CalculateZoomedFontSize(), max_size);
}

std::string GpuDebugMarkerTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const TextBox* text_box = batcher.GetTextBox(id);
  if (text_box == nullptr) {
    return "";
  }

  const TimerInfo& timer_info = text_box->GetTimerInfo();
  CHECK(timer_info.type() == TimerInfo::kGpuDebugMarker);

  std::string marker_text = string_manager_->Get(timer_info.user_data_key()).value_or("");
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
      marker_text, capture_data_->GetThreadName(timer_info.process_id()), timer_info.process_id(),
      capture_data_->GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      orbit_display_formats::GetDisplayTime(TicksToDuration(timer_info.start(), timer_info.end()))
          .c_str());
}

float GpuDebugMarkerTrack::GetYFromTimer(const TimerInfo& timer_info) const {
  uint32_t depth = timer_info.depth();
  if (collapse_toggle_->IsCollapsed()) {
    depth = 0;
  }
  return pos_[1] - layout_->GetTrackTabHeight() - layout_->GetTextBoxHeight() * (depth + 1.f);
}

float GpuDebugMarkerTrack::GetHeight() const {
  bool collapsed = collapse_toggle_->IsCollapsed();
  uint32_t depth = collapsed ? std::min<uint32_t>(1, GetDepth()) : GetDepth();
  return layout_->GetTrackTabHeight() + layout_->GetTextBoxHeight() * depth +
         layout_->GetTrackBottomMargin();
}

bool GpuDebugMarkerTrack::TimerFilter(const TimerInfo& timer_info) const {
  if (collapse_toggle_->IsCollapsed()) {
    return timer_info.depth() == 0;
  }
  return true;
}
