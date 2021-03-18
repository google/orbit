// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>

#include <algorithm>
#include <atomic>
#include <optional>

#include "App.h"
#include "Batcher.h"
#include "CoreUtils.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "ManualInstrumentationManager.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Tracing.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientModel/CaptureData.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "TriangleToggle.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

ThreadTrack::ThreadTrack(TimeGraph* time_graph, TimeGraphLayout* layout, int32_t thread_id,
                         OrbitApp* app, const CaptureData* capture_data)
    : TimerTrack(time_graph, layout, app, capture_data) {
  thread_id_ = thread_id;
  InitializeNameAndLabel(thread_id);

  thread_state_bar_ = std::make_shared<orbit_gl::ThreadStateBar>(app_, time_graph, layout,
                                                                 capture_data, thread_id_, this);

  event_bar_ = std::make_shared<orbit_gl::CallstackThreadBar>(app_, time_graph, layout,
                                                              capture_data, thread_id_, this);
  event_bar_->SetThreadId(thread_id);

  tracepoint_bar_ = std::make_shared<orbit_gl::TracepointThreadBar>(app_, time_graph, layout,
                                                                    capture_data, thread_id_, this);
  SetTrackColor(TimeGraph::GetThreadColor(thread_id));
}

void ThreadTrack::InitializeNameAndLabel(int32_t thread_id) {
  if (thread_id == orbit_base::kAllThreadsOfAllProcessesTid) {
    SetName("All tracepoint events");
    SetLabel("All tracepoint events");
  } else if (thread_id == orbit_base::kAllProcessThreadsTid) {
    // This is the process track.
    const CaptureData& capture_data = app_->GetCaptureData();
    std::string process_name = capture_data.process_name();
    SetName("All Threads");
    const std::string_view all_threads = " (all_threads)";
    SetLabel(process_name.append(all_threads));
    SetNumberOfPrioritizedTrailingCharacters(all_threads.size() - 1);
  } else {
    const std::string& thread_name = time_graph_->GetThreadNameFromTid(thread_id);
    SetName(thread_name);
    std::string tid_str = std::to_string(thread_id);
    std::string track_label = absl::StrFormat("%s [%s]", thread_name, tid_str);
    SetLabel(track_label);
    SetNumberOfPrioritizedTrailingCharacters(tid_str.size() + 2);
  }
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

std::string ThreadTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const TextBox* text_box = batcher.GetTextBox(id);
  if (!text_box || text_box->GetTimerInfo().type() == TimerInfo::kCoreActivity) {
    return "";
  }

  const FunctionInfo* func =
      capture_data_
          ? capture_data_->GetInstrumentedFunctionById(text_box->GetTimerInfo().function_id())
          : nullptr;

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
    function_name = function_utils::GetDisplayName(*func);
  }

  return absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Timing measured through %s instrumentation</i>"
      "<br/><br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Time:</b> %s",
      function_name, is_manual ? "manual" : "dynamic", function_utils::GetLoadedModuleName(*func),
      GetPrettyTime(
          TicksToDuration(text_box->GetTimerInfo().start(), text_box->GetTimerInfo().end())));
}

bool ThreadTrack::IsTimerActive(const TimerInfo& timer_info) const {
  return timer_info.type() == TimerInfo::kIntrospection ||
         timer_info.type() == TimerInfo::kApiEvent ||
         app_->IsFunctionVisible(timer_info.function_id());
}

bool ThreadTrack::IsTrackSelected() const {
  return thread_id_ != orbit_base::kAllProcessThreadsTid &&
         app_->selected_thread_id() == thread_id_;
}

[[nodiscard]] static inline Color ToColor(uint64_t val) {
  return Color((val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
}

[[nodiscard]] static inline std::optional<Color> GetUserColor(const TimerInfo& timer_info,
                                                              const FunctionInfo* function_info) {
  FunctionInfo::OrbitType type = function_info ? function_info->orbit_type() : FunctionInfo::kNone;
  bool manual_instrumentation_timer =
      (type == FunctionInfo::kOrbitTimerStart || type == FunctionInfo::kOrbitTimerStartAsync ||
       timer_info.type() == TimerInfo::kApiEvent);

  if (!manual_instrumentation_timer) {
    return std::nullopt;
  }

  orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
  if (event.color == kOrbitColorAuto) {
    return std::nullopt;
  }

  return ToColor(static_cast<uint64_t>(event.color));
}

Color ThreadTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected,
                                 bool is_highlighted) const {
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

  uint64_t function_id = timer_info.function_id();
  const FunctionInfo* function_info = app_->GetInstrumentedFunction(function_id);
  CHECK(function_info || timer_info.type() == TimerInfo::kIntrospection ||
        timer_info.type() == TimerInfo::kApiEvent);
  std::optional<Color> user_color = GetUserColor(timer_info, function_info);

  Color color = kInactiveColor;
  if (user_color.has_value()) {
    color = user_color.value();
  } else if (timer_info.type() == TimerInfo::kIntrospection) {
    orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
    color = event.color == kOrbitColorAuto ? TimeGraph::GetColor(event.name)
                                           : ToColor(static_cast<uint64_t>(event.color));
  } else {
    color = TimeGraph::GetThreadColor(timer_info.thread_id());
  }

  constexpr uint8_t kOddAlpha = 210;
  if (!(timer_info.depth() & 0x1)) {
    color[3] = kOddAlpha;
  }

  return color;
}

void ThreadTrack::UpdateBoxHeight() {
  box_height_ = layout_->GetTextBoxHeight();
  if (collapse_toggle_->IsCollapsed() && depth_ > 0) {
    box_height_ /= static_cast<float>(depth_);
  }
}

bool ThreadTrack::IsEmpty() const {
  return thread_state_bar_->IsEmpty() && event_bar_->IsEmpty() && tracepoint_bar_->IsEmpty() &&
         (GetNumTimers() == 0);
}

void ThreadTrack::UpdatePositionOfSubtracks() {
  const float thread_state_track_height = layout_->GetThreadStateTrackHeight();
  const float event_track_height = layout_->GetEventTrackHeight();
  const float space_between_subtracks = layout_->GetSpaceBetweenTracksAndThread();

  float current_y = pos_[1];

  thread_state_bar_->SetPos(pos_[0], current_y);
  if (!thread_state_bar_->IsEmpty()) {
    current_y -= (space_between_subtracks + thread_state_track_height);
  }

  event_bar_->SetPos(pos_[0], current_y);
  if (!event_bar_->IsEmpty()) {
    current_y -= (space_between_subtracks + event_track_height);
  }

  tracepoint_bar_->SetPos(pos_[0], current_y);
}

void ThreadTrack::UpdateMinMaxTimestamps() {
  // Tracks in the introspection window don't have capture data.
  if (capture_data_ == nullptr) return;

  min_time_ = std::min(min_time_.load(), capture_data_->GetCallstackData()->min_time());
  max_time_ = std::max(max_time_.load(), capture_data_->GetCallstackData()->max_time());
}

void ThreadTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  TimerTrack::Draw(canvas, picking_mode, z_offset);

  UpdateMinMaxTimestamps();
  UpdatePositionOfSubtracks();

  const float thread_state_track_height = layout_->GetThreadStateTrackHeight();
  const float event_track_height = layout_->GetEventTrackHeight();
  const float tracepoint_track_height = layout_->GetEventTrackHeight();

  if (!thread_state_bar_->IsEmpty()) {
    thread_state_bar_->SetSize(canvas->GetWorldWidth(), thread_state_track_height);
    thread_state_bar_->Draw(canvas, picking_mode, z_offset);
  }

  if (!event_bar_->IsEmpty()) {
    event_bar_->SetSize(canvas->GetWorldWidth(), event_track_height);
    event_bar_->Draw(canvas, picking_mode, z_offset);
  }

  if (!tracepoint_bar_->IsEmpty()) {
    tracepoint_bar_->SetSize(canvas->GetWorldWidth(), tracepoint_track_height);
    tracepoint_bar_->Draw(canvas, picking_mode, z_offset);
  }
}

void ThreadTrack::OnPick(int x, int y) {
  Track::OnPick(x, y);
  app_->set_selected_thread_id(thread_id_);
}

void ThreadTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                   PickingMode picking_mode, float z_offset) {
  UpdatePositionOfSubtracks();

  if (!thread_state_bar_->IsEmpty()) {
    thread_state_bar_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
  if (!event_bar_->IsEmpty()) {
    event_bar_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
  if (!tracepoint_bar_->IsEmpty()) {
    tracepoint_bar_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }

  TimerTrack::UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
}

std::vector<orbit_gl::CaptureViewElement*> ThreadTrack::GetVisibleChildren() {
  std::vector<CaptureViewElement*> result;
  if (!thread_state_bar_->IsEmpty()) {
    result.push_back(thread_state_bar_.get());
  }

  if (!event_bar_->IsEmpty()) {
    result.push_back(event_bar_.get());
  }

  if (!tracepoint_bar_->IsEmpty()) {
    result.push_back(tracepoint_bar_.get());
  }

  return result;
}

void ThreadTrack::SetTrackColor(Color color) {
  absl::MutexLock lock(&mutex_);
  event_bar_->SetColor(color);
  tracepoint_bar_->SetColor(color);
}

void ThreadTrack::SetTimesliceText(const TimerInfo& timer_info, double elapsed_us, float min_x,
                                   float z_offset, TextBox* text_box) {
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Microseconds(elapsed_us));
    text_box->SetElapsedTimeTextLength(time.length());

    const FunctionInfo* func = app_->GetInstrumentedFunction(timer_info.function_id());
    if (func) {
      std::string extra_info = GetExtraInfo(timer_info);
      std::string name;
      if (func->orbit_type() == FunctionInfo::kOrbitTimerStart) {
        auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
        name = api_event.name;
      } else {
        name = function_utils::GetDisplayName(*func);
      }

      std::string text = absl::StrFormat("%s %s %s", name, extra_info.c_str(), time.c_str());

      text_box->SetText(text);
    } else if (timer_info.type() == TimerInfo::kIntrospection) {
      auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
      std::string text = absl::StrFormat("%s %s", api_event.name, time.c_str());
      text_box->SetText(text);
    } else if (timer_info.type() == TimerInfo::kApiEvent) {
      auto api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
      std::string extra_info = GetExtraInfo(timer_info);
      std::string text =
          absl::StrFormat("%s %s %s", api_event.name, extra_info.c_str(), time.c_str());
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
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout_->GetTextOffset(),
      GlCanvas::kZValueBox + z_offset, kTextWhite, text_box->GetElapsedTimeTextLength(),
      layout_->CalculateZoomedFontSize(), max_size);
}

std::string ThreadTrack::GetTooltip() const {
  return "Shows collected samples and timings from dynamically instrumented "
         "functions";
}

float ThreadTrack::GetHeight() const {
  const bool is_collapsed = collapse_toggle_->IsCollapsed();
  const uint32_t collapsed_depth = (GetNumTimers() == 0) ? 0 : 1;
  const uint32_t depth = is_collapsed ? collapsed_depth : GetDepth();

  bool gap_between_tracks_and_timers =
      (!thread_state_bar_->IsEmpty() || !event_bar_->IsEmpty() || !tracepoint_bar_->IsEmpty()) &&
      (depth > 0);
  return GetHeaderHeight() +
         (gap_between_tracks_and_timers ? layout_->GetSpaceBetweenTracksAndThread() : 0) +
         layout_->GetTextBoxHeight() * depth + layout_->GetTrackBottomMargin();
}

float ThreadTrack::GetHeaderHeight() const {
  const float thread_state_track_height = layout_->GetThreadStateTrackHeight();
  const float event_track_height = layout_->GetEventTrackHeight();
  const float tracepoint_track_height = layout_->GetEventTrackHeight();
  const float space_between_subtracks = layout_->GetSpaceBetweenTracksAndThread();

  float header_height = 0.0f;
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
