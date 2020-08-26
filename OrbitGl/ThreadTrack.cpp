// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadTrack.h"

#include "App.h"
#include "Capture.h"
#include "FunctionUtils.h"
#include "GlCanvas.h"
#include "OrbitBase/Profiling.h"
#include "TextBox.h"
#include "TimeGraph.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

ThreadTrack::ThreadTrack(TimeGraph* time_graph, int32_t thread_id)
    : TimerTrack(time_graph), thread_id_(thread_id) {
  event_track_ = std::make_shared<EventTrack>(time_graph);
  event_track_->SetThreadId(thread_id);
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
      Capture::capture_data_.GetSelectedFunction(text_box->GetTimerInfo().function_address());
  CHECK(func != nullptr);

  if (!func) {
    return text_box->GetText();
  }

  std::string function_name;
  bool is_manual = func->type() == orbit_client_protos::FunctionInfo::kOrbitTimerStart;
  if (is_manual) {
    constexpr int kStringArgumentIndex = 0;
    const TimerInfo& timer_info = text_box->GetTimerInfo();
    CHECK(timer_info.registers_size() > kStringArgumentIndex);
    uint64_t string_address = timer_info.registers(kStringArgumentIndex);
    function_name = time_graph_->GetManualInstrumentationString(string_address);
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
  return GOrbitApp->IsFunctionVisible(timer_info.function_address());
}

[[nodiscard]] static inline Color ToColor(uint64_t val) {
  return Color((val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
}

[[nodiscard]] static inline std::optional<Color> GetUserColor(const TimerInfo& timer_info,
                                                              const FunctionInfo& function_info) {
  FunctionInfo::OrbitType type = function_info.type();
  if (type != FunctionInfo::kOrbitTimerStart && type != FunctionInfo::kOrbitTimerStartAsync) {
    return std::optional<Color>{};
  }

  // See Orbit.h for more information about the manual instrumentation API.
  const int kColorArgumentIndex = type == FunctionInfo::kOrbitTimerStart ? 1 : 2;
  constexpr uint64_t kColorAuto = 0x00000001;
  CHECK(timer_info.registers_size() > kColorArgumentIndex);
  uint64_t color_arg = timer_info.registers(kColorArgumentIndex);
  return color_arg != kColorAuto ? ToColor(color_arg) : std::optional<Color>{};
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
  const FunctionInfo* function_info = Capture::capture_data_.GetSelectedFunction(address);
  CHECK(function_info);
  std::optional<Color> user_color = GetUserColor(timer_info, *function_info);

  Color color = user_color.has_value() ? user_color.value()
                                       : time_graph_->GetThreadColor(timer_info.thread_id());

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

bool ThreadTrack::IsEmpty() const { return (GetNumTimers() == 0) && (event_track_->IsEmpty()); }

void ThreadTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  TimerTrack::Draw(canvas, picking_mode);

  float event_track_height = time_graph_->GetLayout().GetEventTrackHeight();
  event_track_->SetPos(m_Pos[0], m_Pos[1]);
  event_track_->SetSize(canvas->GetWorldWidth(), event_track_height);
  event_track_->Draw(canvas, picking_mode);
}

void ThreadTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) {
  event_track_->SetPos(m_Pos[0], m_Pos[1]);
  event_track_->UpdatePrimitives(min_tick, max_tick, picking_mode);
  TimerTrack::UpdatePrimitives(min_tick, max_tick, picking_mode);
}

void ThreadTrack::SetEventTrackColor(Color color) {
  ScopeLock lock(mutex_);
  event_track_->SetColor(color);
}

void ThreadTrack::SetTimesliceText(const TimerInfo& timer_info, double elapsed_us, float min_x,
                                   TextBox* text_box) {
  TimeGraphLayout layout = time_graph_->GetLayout();
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Microseconds(elapsed_us));
    const FunctionInfo* func =
        Capture::capture_data_.GetSelectedFunction(timer_info.function_address());

    text_box->SetElapsedTimeTextLength(time.length());

    if (func) {
      std::string extra_info = GetExtraInfo(timer_info);
      std::string name;
      if (func->type() == FunctionInfo::kOrbitTimerStart) {
        name = time_graph_->GetManualInstrumentationString(timer_info.registers(0));
        if (name.empty()) {
          // The remote string hasn't been retrieved yet,
          // early out and try again on next update.
          return;
        }
      } else {
        name = FunctionUtils::GetDisplayName(*func);
      }

      std::string text = absl::StrFormat("%s %s %s", name, extra_info.c_str(), time.c_str());

      text_box->SetText(text);
    } else if (timer_info.type() == TimerInfo::kIntrospection) {
      std::string text = absl::StrFormat(
          "%s %s", time_graph_->GetStringManager()->Get(timer_info.user_data_key()).value_or(""),
          time.c_str());
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
      GlCanvas::Z_VALUE_TEXT, kTextWhite, text_box->GetElapsedTimeTextLength(), max_size);
}

std::string ThreadTrack::GetTooltip() const {
  return "Shows collected samples and timings from dynamically instrumented "
         "functions";
}
