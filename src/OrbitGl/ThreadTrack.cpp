// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/ThreadTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ScopeId.h"
#include "ClientProtos/capture_data.pb.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Typedef.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/ThreadColor.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimerTrack.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/TrackHeader.h"
#include "OrbitGl/Viewport.h"

using orbit_client_data::FunctionInfo;
using orbit_client_data::ScopeId;

using orbit_gl::PickingUserData;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

using orbit_client_protos::TimerInfo;

ThreadTrack::ThreadTrack(CaptureViewElement* parent,
                         const orbit_gl::TimelineInfoInterface* timeline_info,
                         orbit_gl::Viewport* viewport, TimeGraphLayout* layout, uint32_t thread_id,
                         OrbitApp* app, const orbit_client_data::ModuleManager* module_manager,
                         const orbit_client_data::CaptureData* capture_data,
                         orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider)
    : TimerTrack(parent, timeline_info, viewport, layout, app, module_manager, capture_data,
                 nullptr),
      thread_id_{thread_id},
      thread_track_data_provider_{thread_track_data_provider} {
  Color color = orbit_gl::GetThreadColor(thread_id);
  thread_state_bar_ = std::make_shared<orbit_gl::ThreadStateBar>(
      this, app_, timeline_info_, viewport, layout, module_manager, capture_data, thread_id);

  event_bar_ = std::make_shared<orbit_gl::CallstackThreadBar>(
      this, app_, timeline_info_, viewport, layout, module_manager, capture_data, thread_id);

  tracepoint_bar_ = std::make_shared<orbit_gl::TracepointThreadBar>(
      this, app_, timeline_info_, viewport, layout, module_manager, capture_data, thread_id);
}

std::string ThreadTrack::GetName() const {
  auto thread_id = GetThreadId();
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    return "All tracepoint events";
  } else if (thread_id == orbit_base::kAllProcessThreadsTid) {
    return "All Threads";
  }
  return capture_data_->GetThreadName(thread_id);
}

constexpr std::string_view kAllThreads = " (all threads)";
std::string ThreadTrack::GetLabel() const {
  auto thread_id = GetThreadId();
  auto name = GetName();
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    return name;
  } else if (thread_id == orbit_base::kAllProcessThreadsTid) {
    std::string process_name = capture_data_->process_name();
    return process_name.append(kAllThreads);
  }
  return absl::StrFormat("%s [%d]", name, thread_id);
}

// Make sure some trailing characters have higher priority if there is no space for the full label.
int ThreadTrack::GetNumberOfPrioritizedTrailingCharacters() const {
  auto thread_id = GetThreadId();
  if (GetThreadId() == orbit_base::kAllThreadsOfAllProcessesTid) {
    return 0;
  } else if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
    // Example: proc... all_threads)
    return kAllThreads.size() - 1;
  }
  // Example: thread_na... [85023]
  return std::to_string(thread_id).size() + 2;
}

const TimerInfo* ThreadTrack::GetLeft(const TimerInfo& timer_info) const {
  return thread_track_data_provider_->GetLeft(timer_info);
}

const TimerInfo* ThreadTrack::GetRight(const TimerInfo& timer_info) const {
  return thread_track_data_provider_->GetRight(timer_info);
}

const TimerInfo* ThreadTrack::GetUp(const TimerInfo& timer_info) const {
  return thread_track_data_provider_->GetUp(timer_info);
}

const TimerInfo* ThreadTrack::GetDown(const TimerInfo& timer_info) const {
  return thread_track_data_provider_->GetDown(timer_info);
}

std::string ThreadTrack::GetBoxTooltip(const PrimitiveAssembler& primitive_assembler,
                                       PickingId id) const {
  const TimerInfo* timer_info = primitive_assembler.GetTimerInfo(id);
  if (timer_info == nullptr || timer_info->type() == TimerInfo::kCoreActivity) {
    return "";
  }

  const FunctionInfo* func = capture_data_->GetFunctionInfoById(timer_info->function_id());

  std::string label;
  bool is_manual = timer_info->type() == TimerInfo::kApiScope;

  if (func == nullptr && !is_manual) {
    return GetTimesliceText(*timer_info);
  }

  if (is_manual) {
    label = timer_info->api_scope_name();
  } else {
    label = func->pretty_name();
  }

  std::string module_name = orbit_client_data::kUnknownFunctionOrModuleName;
  std::string function_name = orbit_client_data::kUnknownFunctionOrModuleName;
  if (func != nullptr) {
    module_name = std::filesystem::path(func->module_path()).filename().string();
    function_name = label;
  } else if (timer_info->address_in_function() != 0) {
    module_name = std::filesystem::path(
                      orbit_client_data::GetModulePathByAddress(*module_manager_, *capture_data_,
                                                                timer_info->address_in_function()))
                      .filename()
                      .string();

    function_name = orbit_client_data::GetFunctionNameByAddress(*module_manager_, *capture_data_,
                                                                timer_info->address_in_function());
  }

  std::string result = absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Timing measured through %s instrumentation</i>"
      "<br/><br/>"
      "<b>Function:</b> %s<br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Time:</b> %s",
      label, is_manual ? "manual" : "dynamic", function_name, module_name,
      orbit_display_formats::GetDisplayTime(
          TicksToDuration(timer_info->start(), timer_info->end())));

  if (timer_info->group_id() != kOrbitDefaultGroupId) {
    result += absl::StrFormat("<br/><b>Group Id:</b> %lu", timer_info->group_id());
  }

  return result;
}

bool ThreadTrack::IsTimerActive(const TimerInfo& timer_info) const {
  if (!app_->HasCaptureData()) return TimerTrack::IsTimerActive(timer_info);
  const std::optional<ScopeId> scope_id = app_->GetCaptureData().ProvideScopeId(timer_info);
  return scope_id.has_value() ? app_->IsScopeVisible(scope_id.value()) : false;
}

bool ThreadTrack::IsTrackSelected() const {
  return GetThreadId() != orbit_base::kAllProcessThreadsTid &&
         app_->selected_thread_id() == GetThreadId();
}

[[nodiscard]] static std::optional<Color> GetUserColor(const TimerInfo& timer_info) {
  if (timer_info.type() == TimerInfo::kApiScope) {
    if (!timer_info.has_color()) {
      return std::nullopt;
    }
    ORBIT_CHECK(timer_info.color().red() < 256);
    ORBIT_CHECK(timer_info.color().green() < 256);
    ORBIT_CHECK(timer_info.color().blue() < 256);
    ORBIT_CHECK(timer_info.color().alpha() < 256);
    return Color(static_cast<uint8_t>(timer_info.color().red()),
                 static_cast<uint8_t>(timer_info.color().green()),
                 static_cast<uint8_t>(timer_info.color().blue()),
                 static_cast<uint8_t>(timer_info.color().alpha()));
  }

  return std::nullopt;
}

float ThreadTrack::GetDefaultBoxHeight() const {
  auto box_height = layout_->GetTextBoxHeight();
  if (IsCollapsed() && GetDepth() > 0) {
    return box_height / static_cast<float>(GetDepth());
  }
  return box_height;
}

Color ThreadTrack::GetTimerColor(const TimerInfo& timer_info, const internal::DrawData& draw_data) {
  const std::optional<ScopeId> scope_id =
      app_->HasCaptureData() ? app_->GetCaptureData().ProvideScopeId(timer_info) : std::nullopt;
  const uint64_t group_id = timer_info.group_id();
  const bool is_selected = &timer_info == draw_data.selected_timer;
  const bool is_scope_id_highlighted =
      scope_id.has_value() && scope_id.value() == draw_data.highlighted_scope_id;
  const bool is_group_id_highlighted =
      group_id != kOrbitDefaultGroupId && group_id == draw_data.highlighted_group_id;
  const bool is_highlighted = !is_selected && (is_scope_id_highlighted || is_group_id_highlighted);
  return GetTimerColor(timer_info, is_selected, is_highlighted, draw_data);
}

Color ThreadTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected, bool is_highlighted,
                                 const internal::DrawData& /*draw_data*/) const {
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

  std::optional<Color> user_color = GetUserColor(timer_info);

  Color color;
  if (user_color.has_value()) {
    color = user_color.value();
  } else {
    color = orbit_gl::GetThreadColor(timer_info.thread_id());
  }

  constexpr float kOddRowColorMultiplier = 210.f / 255.f;
  if ((timer_info.depth() & 0x1) == 0) {
    // We are slightly alternating the colors for thread timers based on their depth.
    for (int i = 0; i < 3; ++i) {
      color[i] = static_cast<char>(color[i] * kOddRowColorMultiplier);
    }
  }

  return color;
}

bool ThreadTrack::IsEmpty() const {
  return thread_state_bar_->IsEmpty() && event_bar_->IsEmpty() && tracepoint_bar_->IsEmpty() &&
         thread_track_data_provider_->IsEmpty(thread_id_);
}

void ThreadTrack::UpdatePositionOfSubtracks() {
  const float thread_state_track_height = layout_->GetThreadStateTrackHeight();
  const float event_track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const float space_between_subtracks = layout_->GetSpaceBetweenThreadPanes();

  const Vec2 pos = GetPos();
  float current_y =
      header_->GetPos()[1] + header_->GetHeight() + layout_->GetTrackContentTopMargin();

  thread_state_bar_->SetPos(pos[0], current_y);
  if (thread_state_bar_->ShouldBeRendered()) {
    current_y += (space_between_subtracks + thread_state_track_height);
  }

  event_bar_->SetPos(pos[0], current_y);
  if (event_bar_->ShouldBeRendered()) {
    current_y += (space_between_subtracks + event_track_height);
  }

  tracepoint_bar_->SetPos(pos[0], current_y);
}

void ThreadTrack::SelectTrack() { app_->set_selected_thread_id(GetThreadId()); }

std::vector<orbit_gl::CaptureViewElement*> ThreadTrack::GetAllChildren() const {
  auto result = Track::GetAllChildren();
  result.insert(result.end(), {thread_state_bar_.get(), event_bar_.get(), tracepoint_bar_.get()});
  return result;
}

std::string ThreadTrack::GetTimesliceText(const TimerInfo& timer_info) const {
  std::string time = GetDisplayTime(timer_info);

  const FunctionInfo* func = capture_data_->GetFunctionInfoById(timer_info.function_id());
  if (func != nullptr) {
    std::string extra_info = GetExtraInfo(timer_info);
    const std::string& name = func->pretty_name();
    return absl::StrFormat("%s %s %s", name, extra_info.c_str(), time);
  }
  if (timer_info.type() == TimerInfo::kApiScope) {
    std::string extra_info = GetExtraInfo(timer_info);
    return absl::StrFormat("%s %s %s", timer_info.api_scope_name(), extra_info.c_str(), time);
  }

  ORBIT_ERROR("Unexpected case in ThreadTrack::SetTimesliceText: function=\"%s\", type=%d",
              func->pretty_name(), static_cast<int>(timer_info.type()));
  return "";
}

std::string ThreadTrack::GetTooltip() const {
  if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
    return "Shows collected samples for all threads of process " + capture_data_->process_name() +
           " " + std::to_string(capture_data_->process_id());
  }
  return "Shows collected samples and timings from dynamically instrumented "
         "functions";
}

float ThreadTrack::GetHeight() const {
  const uint32_t depth = IsCollapsed() ? std::min<uint32_t>(1, GetDepth()) : GetDepth();

  bool gap_between_tracks_and_timers =
      (!thread_state_bar_->IsEmpty() || !event_bar_->IsEmpty() || !tracepoint_bar_->IsEmpty()) &&
      (depth > 0);
  return GetHeightAboveTimers() +
         (gap_between_tracks_and_timers ? layout_->GetSpaceBetweenThreadPanes() : 0) +
         layout_->GetTextBoxHeight() * depth + layout_->GetTrackContentBottomMargin();
}

float ThreadTrack::GetHeightAboveTimers() const {
  const float thread_state_track_height = layout_->GetThreadStateTrackHeight();
  const float event_track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const float tracepoint_track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const float space_between_subtracks = layout_->GetSpaceBetweenThreadPanes();

  float header_height = header_->GetHeight() + layout_->GetTrackContentTopMargin();
  int track_count = 0;
  if (!thread_state_bar_->IsEmpty()) {
    header_height += thread_state_track_height;
    ++track_count;
  }

  if (!event_bar_->IsEmpty()) {
    header_height += event_track_height;
    ++track_count;
  }

  if (!tracepoint_bar_->IsEmpty()) {
    header_height += tracepoint_track_height;
    ++track_count;
  }

  header_height += std::max(0, track_count - 1) * space_between_subtracks;
  return header_height;
}

float ThreadTrack::GetYFromDepth(uint32_t depth) const {
  bool gap_between_tracks_and_timers =
      !thread_state_bar_->IsEmpty() || !event_bar_->IsEmpty() || !tracepoint_bar_->IsEmpty();
  return GetPos()[1] + GetHeightAboveTimers() +
         (gap_between_tracks_and_timers ? layout_->GetSpaceBetweenThreadPanes() : 0) +
         GetDefaultBoxHeight() * static_cast<float>(depth);
}

// TODO (http://b/202110356): Erase this function when erasing all OnTimer() in TimerTracks.
void ThreadTrack::OnTimer(const TimerInfo& timer_info) {
  thread_track_data_provider_->AddTimer(timer_info);
}

// We minimize overdraw when drawing lines for small events by discarding events that would just
// draw over an already drawn pixel line. When zoomed in enough that all events are drawn as boxes,
// this has no effect. When zoomed  out, many events will be discarded quickly.
void ThreadTrack::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                     TextRenderer& text_renderer, uint64_t min_tick,
                                     uint64_t max_tick, PickingMode /*picking_mode*/) {
  // TODO(b/203181055): The parent class already provides an implementation, but this is completely
  // ignored because ThreadTrack uses the ScopeTree, and TimerTrack doesn't.
  // TimerTrack::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
  // picking_mode);
  ORBIT_SCOPE_WITH_COLOR("ThreadTrack::DoUpdatePrimitives", kOrbitColorYellow);
  visible_timer_count_ = 0;

  const internal::DrawData draw_data =
      GetDrawData(min_tick, max_tick, GetPos()[0], GetWidth(), &primitive_assembler, timeline_info_,
                  viewport_, IsCollapsed(), app_->selected_timer(), app_->GetScopeIdToHighlight(),
                  app_->GetGroupIdToHighlight(), app_->GetHistogramSelectionRange());

  uint64_t resolution_in_pixels = draw_data.viewport->WorldToScreen({draw_data.track_width, 0})[0];
  for (uint32_t depth = 0; depth < GetDepth(); depth++) {
    float world_timer_y = GetYFromDepth(depth);

    for (const TimerInfo* timer_info : thread_track_data_provider_->GetTimersAtDepthDiscretized(
             thread_id_, depth, resolution_in_pixels, min_tick, max_tick)) {
      ++visible_timer_count_;

      Color color = GetTimerColor(*timer_info, draw_data);
      std::unique_ptr<PickingUserData> user_data =
          CreatePickingUserData(primitive_assembler, *timer_info);

      auto box_height = GetDefaultBoxHeight();
      const auto [pos_x, size_x] =
          timeline_info_->GetBoxPosXAndWidthFromTicks(timer_info->start(), timer_info->end());
      const Vec2 pos = {pos_x, world_timer_y};
      const Vec2 size = {size_x, box_height};

      if (!IsCollapsed() && BoxHasRoomForText(text_renderer, size[0])) {
        DrawTimesliceText(text_renderer, *timer_info, draw_data.track_start_x, pos, size);
      }
      primitive_assembler.AddShadedBox(pos, size, draw_data.z, color, std::move(user_data));
      if (ShouldHaveBorder(timer_info, draw_data.histogram_selection_range, size[0])) {
        primitive_assembler.AddQuadBorder(MakeBox(pos, size), GlCanvas::kZValueBoxBorder,
                                          TimerTrack::kBoxBorderColor,
                                          CreatePickingUserData(primitive_assembler, *timer_info));
      }
    }
  }
}
