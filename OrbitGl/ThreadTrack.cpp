// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadTrack.h"

#include "../Orbit.h"
#include "App.h"
#include "GlCanvas.h"
#include "ManualInstrumentationManager.h"
#include "OrbitBase/Profiling.h"
#include "OrbitClientData/FunctionUtils.h"
#include "TextBox.h"
#include "TimeGraph.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

ThreadTrack::ThreadTrack(TimeGraph* time_graph, int32_t thread_id) : TimerTrack(time_graph) {
  thread_id_ = thread_id;

  thread_state_track_ = std::make_shared<ThreadStateTrack>(time_graph, thread_id);

  event_track_ = std::make_shared<EventTrack>(time_graph);
  event_track_->SetThreadId(thread_id);

  tracepoint_track_ = std::make_shared<TracepointTrack>(time_graph, thread_id);
}

const TextBox* ThreadTrack::GetLeft(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  if (timer_info.thread_id() == thread_id_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer_info.depth());
    if (timers) return timers->GetElementBefore(text_box);
  }
  return nullptr;
}

const TextBox* ThreadTrack::GetRight(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  if (timer_info.thread_id() == thread_id_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer_info.depth());
    if (timers) return timers->GetElementAfter(text_box);
  }
  return nullptr;
}

std::string ThreadTrack::GetBoxTooltip(PickingId id) const {
  const TextBox* text_box = time_graph_->GetBatcher().GetTextBox(id);
  if (!text_box || text_box->GetTimerInfo().type() == TimerInfo::kCoreActivity) {
    return "";
  }

  const FunctionInfo* func =
      GOrbitApp->GetCaptureData().GetSelectedFunction(text_box->GetTimerInfo().function_address());

  if (!func) {
    return text_box->GetText();
  }

  std::string function_name;
  bool is_manual = func->orbit_type() == orbit_client_protos::FunctionInfo::kOrbitTimerStart;
  if (is_manual) {
    const TimerInfo& timer_info = text_box->GetTimerInfo();
    auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
    function_name = api_event.name;
  } else {
    function_name = FunctionUtils::GetDisplayName(*func);
  }

  return absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Timing measured through %s instrumentation</i>"
      "<br/><br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Time:</b> %s",
      function_name, is_manual ? "manual" : "dynamic", FunctionUtils::GetLoadedModuleName(*func),
      GetPrettyTime(
          TicksToDuration(text_box->GetTimerInfo().start(), text_box->GetTimerInfo().end())));
}

bool ThreadTrack::IsTimerActive(const TimerInfo& timer_info) const {
  return timer_info.type() == TimerInfo::kIntrospection ||
         GOrbitApp->IsFunctionVisible(timer_info.function_address());
}

bool ThreadTrack::IsTrackSelected() const {
  return thread_id_ != SamplingProfiler::kAllThreadsFakeTid &&
         GOrbitApp->selected_thread_id() == thread_id_;
}

[[nodiscard]] static inline Color ToColor(uint64_t val) {
  return Color((val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
}

[[nodiscard]] static inline std::optional<Color> GetUserColor(const TimerInfo& timer_info,
                                                              const FunctionInfo& function_info) {
  FunctionInfo::OrbitType type = function_info.orbit_type();
  if (type != FunctionInfo::kOrbitTimerStart && type != FunctionInfo::kOrbitTimerStartAsync) {
    return std::nullopt;
  }

  orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
  if (event.color == orbit::Color::kAuto) {
    return std::nullopt;
  }

  return ToColor(static_cast<uint64_t>(event.color));
}

Color ThreadTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected) const {
  const Color kInactiveColor(100, 100, 100, 255);
  const Color kSelectionColor(0, 128, 255, 255);
  if (is_selected) {
    return kSelectionColor;
  } else if (!IsTimerActive(timer_info)) {
    return kInactiveColor;
  }

  uint64_t address = timer_info.function_address();
  const FunctionInfo* function_info = GOrbitApp->GetCaptureData().GetSelectedFunction(address);
  std::optional<Color> user_color =
      function_info ? GetUserColor(timer_info, *function_info) : std::nullopt;

  Color color = kInactiveColor;
  if (user_color.has_value()) {
    color = user_color.value();
  } else if (timer_info.type() == TimerInfo::kIntrospection) {
    orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
    color = event.color == orbit::Color::kAuto ? time_graph_->GetColor(event.name)
                                               : ToColor(static_cast<uint64_t>(event.color));
  } else {
    color = time_graph_->GetThreadColor(timer_info.thread_id());
  }

  constexpr uint8_t kOddAlpha = 210;
  if (!(timer_info.depth() & 0x1)) {
    color[3] = kOddAlpha;
  }

  return color;
}

void ThreadTrack::UpdateBoxHeight() {
  box_height_ = time_graph_->GetLayout().GetTextBoxHeight();
  if (collapse_toggle_->IsCollapsed() && depth_ > 0) {
    box_height_ /= static_cast<float>(depth_);
  }
}

bool ThreadTrack::IsEmpty() const {
  return thread_state_track_->IsEmpty() && event_track_->IsEmpty() &&
         tracepoint_track_->IsEmpty() && (GetNumTimers() == 0);
}

void ThreadTrack::UpdatePositionOfSubtracks() {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const float thread_state_track_height = layout.GetThreadStateTrackHeight();
  const float event_track_height = layout.GetEventTrackHeight();
  const float space_between_subtracks = layout.GetSpaceBetweenTracksAndThread();

  float current_y = pos_[1];

  thread_state_track_->SetPos(pos_[0], current_y);
  if (!thread_state_track_->IsEmpty()) {
    current_y -= (space_between_subtracks + thread_state_track_height);
  }

  event_track_->SetPos(pos_[0], current_y);
  if (!event_track_->IsEmpty()) {
    current_y -= (space_between_subtracks + event_track_height);
  }

  tracepoint_track_->SetPos(pos_[0], current_y);
}

void ThreadTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  TimerTrack::Draw(canvas, picking_mode);

  UpdatePositionOfSubtracks();

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const float thread_state_track_height = layout.GetThreadStateTrackHeight();
  const float event_track_height = layout.GetEventTrackHeight();
  const float tracepoint_track_height = layout.GetEventTrackHeight();

  if (!thread_state_track_->IsEmpty()) {
    thread_state_track_->SetSize(canvas->GetWorldWidth(), thread_state_track_height);
    thread_state_track_->Draw(canvas, picking_mode);
  }

  if (!event_track_->IsEmpty()) {
    event_track_->SetSize(canvas->GetWorldWidth(), event_track_height);
    event_track_->Draw(canvas, picking_mode);
  }

  if (!tracepoint_track_->IsEmpty()) {
    tracepoint_track_->SetSize(canvas->GetWorldWidth(), tracepoint_track_height);
    tracepoint_track_->Draw(canvas, picking_mode);
  }
}

void ThreadTrack::OnPick(int /*x*/, int /*y*/) { GOrbitApp->set_selected_thread_id(thread_id_); }

void ThreadTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) {
  UpdatePositionOfSubtracks();

  if (!thread_state_track_->IsEmpty()) {
    thread_state_track_->UpdatePrimitives(min_tick, max_tick, picking_mode);
  }
  if (!event_track_->IsEmpty()) {
    event_track_->UpdatePrimitives(min_tick, max_tick, picking_mode);
  }
  if (!tracepoint_track_->IsEmpty()) {
    tracepoint_track_->UpdatePrimitives(min_tick, max_tick, picking_mode);
  }

  TimerTrack::UpdatePrimitives(min_tick, max_tick, picking_mode);
}

void ThreadTrack::SetTrackColor(Color color) {
  ScopeLock lock(mutex_);
  event_track_->SetColor(color);
  tracepoint_track_->SetColor(color);
}

void ThreadTrack::SetTimesliceText(const TimerInfo& timer_info, double elapsed_us, float min_x,
                                   TextBox* text_box) {
  TimeGraphLayout layout = time_graph_->GetLayout();
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Microseconds(elapsed_us));
    const FunctionInfo* func =
        GOrbitApp->GetCaptureData().GetSelectedFunction(timer_info.function_address());

    text_box->SetElapsedTimeTextLength(time.length());

    if (func) {
      std::string extra_info = GetExtraInfo(timer_info);
      std::string name;
      if (func->orbit_type() == FunctionInfo::kOrbitTimerStart) {
        auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
        name = api_event.name;
      } else {
        name = FunctionUtils::GetDisplayName(*func);
      }

      std::string text = absl::StrFormat("%s %s %s", name, extra_info.c_str(), time.c_str());

      text_box->SetText(text);
    } else if (timer_info.type() == TimerInfo::kIntrospection) {
      auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
      std::string text = absl::StrFormat("%s %s", api_event.name, time.c_str());
      text_box->SetText(text);
    } else {
      ERROR(
          "Unexpected case in ThreadTrack::SetTimesliceText, function=\"%s\", "
          "type=%d",
          func->name(), static_cast<int>(timer_info.type()));
    }
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout.GetTextOffset(),
      GlCanvas::kZValueText, kTextWhite, text_box->GetElapsedTimeTextLength(),
      time_graph_->CalculateZoomedFontSize(), max_size);
}

std::string ThreadTrack::GetTooltip() const {
  return "Shows collected samples and timings from dynamically instrumented "
         "functions";
}

float ThreadTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  const bool is_collapsed = collapse_toggle_->IsCollapsed();
  const uint32_t collapsed_depth = (GetNumTimers() == 0) ? 0 : 1;
  const uint32_t depth = is_collapsed ? collapsed_depth : GetDepth();

  bool gap_between_tracks_and_timers =
      (!thread_state_track_->IsEmpty() || !event_track_->IsEmpty() ||
       !tracepoint_track_->IsEmpty()) &&
      (depth > 0);
  return GetHeaderHeight() +
         (gap_between_tracks_and_timers ? layout.GetSpaceBetweenTracksAndThread() : 0) +
         layout.GetTextBoxHeight() * depth + layout.GetTrackBottomMargin();
}

float ThreadTrack::GetHeaderHeight() const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const float thread_state_track_height = layout.GetThreadStateTrackHeight();
  const float event_track_height = layout.GetEventTrackHeight();
  const float tracepoint_track_height = layout.GetEventTrackHeight();
  const float space_between_subtracks = layout.GetSpaceBetweenTracksAndThread();

  float header_height = 0.0f;
  int track_count = 0;
  if (!thread_state_track_->IsEmpty()) {
    header_height += thread_state_track_height;
    ++track_count;
  }

  if (!event_track_->IsEmpty()) {
    header_height += event_track_height;
    ++track_count;
  }

  if (!tracepoint_track_->IsEmpty()) {
    header_height += tracepoint_track_height;
    ++track_count;
  }

  header_height += std::max(0, track_count - 1) * space_between_subtracks;
  return header_height;
}
