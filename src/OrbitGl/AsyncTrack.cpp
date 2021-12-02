// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AsyncTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <ctime>

#include "App.h"
#include "Batcher.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionUtils.h"
#include "ClientProtos/capture_data.pb.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "Introspection/Introspection.h"
#include "ManualInstrumentationManager.h"
#include "OrbitBase/Logging.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "TriangleToggle.h"
#include "Viewport.h"

using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::InstrumentedFunction;

AsyncTrack::AsyncTrack(CaptureViewElement* parent,
                       const orbit_gl::TimelineInfoInterface* timeline_info,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                       OrbitApp* app, const orbit_client_data::CaptureData* capture_data,
                       orbit_client_data::TimerData* timer_data)
    : TimerTrack(parent, timeline_info, viewport, layout, app, capture_data, timer_data),
      name_(std::move(name)) {}

[[nodiscard]] std::string AsyncTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const TimerInfo* timer_info = batcher.GetTimerInfo(id);
  if (timer_info == nullptr) return "";
  auto* manual_inst_manager = app_->GetManualInstrumentationManager();

  CHECK(timer_info->type() == TimerInfo::kApiScopeAsync);

  uint64_t event_id = timer_info->api_async_scope_id();
  std::string label = manual_inst_manager->GetString(event_id);

  std::string function_name =
      capture_data_->GetFunctionNameByAddress(timer_info->address_in_function());

  std::string module_name = orbit_client_data::CaptureData::kUnknownFunctionOrModuleName;
  if (timer_info->address_in_function() != 0) {
    const orbit_client_data::ModuleData* module =
        capture_data_->FindModuleByAddress(timer_info->address_in_function());
    if (module != nullptr) {
      module_name = module->name();
    }
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

void AsyncTrack::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                    uint64_t min_tick, uint64_t max_tick,
                                    PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("AsyncTrack::DoUpdatePrimitives", kOrbitColorDeepPurple);
  TimerTrack::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);
}

float AsyncTrack::GetDefaultBoxHeight() const {
  auto box_height = layout_->GetTextBoxHeight();
  if (collapse_toggle_->IsCollapsed() && GetDepth() > 0) {
    return box_height / static_cast<float>(GetDepth());
  }
  return box_height;
}

std::string AsyncTrack::GetTimesliceText(const TimerInfo& timer_info) const {
  CHECK(timer_info.type() == TimerInfo::kApiScopeAsync);
  std::string time = GetDisplayTime(timer_info);
  uint64_t event_id = timer_info.api_async_scope_id();
  std::string name = app_->GetManualInstrumentationManager()->GetString(event_id);
  return absl::StrFormat("%s %s", name, time);
}

Color AsyncTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected, bool is_highlighted,
                                const internal::DrawData& /*draw_data*/) const {
  CHECK(timer_info.type() == TimerInfo::kApiScopeAsync);
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
