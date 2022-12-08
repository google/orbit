// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AsyncTrack.h"

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientProtos/capture_data.pb.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/ManualInstrumentationManager.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

using orbit_client_protos::TimerInfo;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

AsyncTrack::AsyncTrack(CaptureViewElement* parent,
                       const orbit_gl::TimelineInfoInterface* timeline_info,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                       OrbitApp* app, const orbit_client_data::ModuleManager* module_manager,
                       const orbit_client_data::CaptureData* capture_data,
                       orbit_client_data::TimerData* timer_data)
    : TimerTrack(parent, timeline_info, viewport, layout, app, module_manager, capture_data,
                 timer_data),
      name_(std::move(name)) {}

[[nodiscard]] std::string AsyncTrack::GetBoxTooltip(const PrimitiveAssembler& primitive_assembler,
                                                    PickingId id) const {
  const TimerInfo* timer_info = primitive_assembler.GetTimerInfo(id);
  if (timer_info == nullptr) return "";
  auto* manual_inst_manager = app_->GetManualInstrumentationManager();

  ORBIT_CHECK(timer_info->type() == TimerInfo::kApiScopeAsync);

  uint64_t event_id = timer_info->api_async_scope_id();
  std::string label = manual_inst_manager->GetString(event_id);

  std::string function_name = orbit_client_data::GetFunctionNameByAddress(
      *module_manager_, *capture_data_, timer_info->address_in_function());

  std::string module_name = orbit_client_data::kUnknownFunctionOrModuleName;
  if (timer_info->address_in_function() != 0) {
    module_name = std::filesystem::path(
                      orbit_client_data::GetModulePathByAddress(*module_manager_, *capture_data_,
                                                                timer_info->address_in_function()))
                      .filename()
                      .string();
  }

  return absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Timing measured through manual instrumentation</i>"
      "<br/><br/>"
      "<b>Function:</b> %s<br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Time:</b> %s",
      label, function_name, module_name,
      orbit_display_formats::GetDisplayTime(
          TicksToDuration(timer_info->start(), timer_info->end())));
}

void AsyncTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  // Find the first row that that can receive the new timeslice with no overlap.
  // If none of the existing rows works, add a new row.
  uint32_t depth = 0;
  while (max_span_time_by_depth_[depth] > timer_info.start()) ++depth;
  max_span_time_by_depth_[depth] = timer_info.end();

  orbit_client_protos::TimerInfo new_timer_info = timer_info;
  new_timer_info.set_depth(depth);
  TimerTrack::OnTimer(new_timer_info);
}

float AsyncTrack::GetHeight() const {
  uint32_t collapsed_depth = std::min<uint32_t>(1, GetDepth());
  uint32_t depth = IsCollapsed() ? collapsed_depth : GetDepth();
  return GetHeightAboveTimers() + layout_->GetTextBoxHeight() * depth +
         layout_->GetTrackContentBottomMargin();
}

void AsyncTrack::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                    TextRenderer& text_renderer, uint64_t min_tick,
                                    uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("AsyncTrack::DoUpdatePrimitives", kOrbitColorDeepPurple);
  TimerTrack::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                 picking_mode);
}

float AsyncTrack::GetDefaultBoxHeight() const {
  auto box_height = layout_->GetTextBoxHeight();
  if (IsCollapsed() && GetDepth() > 0) {
    return box_height / static_cast<float>(GetDepth());
  }
  return box_height;
}

std::string AsyncTrack::GetTimesliceText(const TimerInfo& timer_info) const {
  ORBIT_CHECK(timer_info.type() == TimerInfo::kApiScopeAsync);
  std::string time = GetDisplayTime(timer_info);
  uint64_t event_id = timer_info.api_async_scope_id();
  std::string name = app_->GetManualInstrumentationManager()->GetString(event_id);
  return absl::StrFormat("%s %s", name, time);
}

Color AsyncTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected, bool is_highlighted,
                                const internal::DrawData& /*draw_data*/) const {
  ORBIT_CHECK(timer_info.type() == TimerInfo::kApiScopeAsync);
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

  uint64_t event_id = timer_info.api_async_scope_id();
  ;
  std::string name = app_->GetManualInstrumentationManager()->GetString(event_id);
  Color color = TimeGraph::GetColor(name);

  constexpr uint8_t kOddAlpha = 210;
  if ((timer_info.depth() & 0x1) == 0u) {
    color[3] = kOddAlpha;
  }

  return color;
}
