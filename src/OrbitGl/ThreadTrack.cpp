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

bool ThreadTrack::render_with_nodes_ = true;

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

void ThreadTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  UpdateDepth(timer_info.depth() + 1);

  if (process_id_ == -1) {
    process_id_ = timer_info.process_id();
  }

  TextBox text_box(Vec2(0, 0), Vec2(0, 0), "");
  text_box.SetTimerInfo(timer_info);

  std::shared_ptr<TimerChain> timer_chain = timers_[timer_info.depth()];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<TimerChain>();
    timers_[timer_info.depth()] = timer_chain;
  }
  timer_chain->push_back(text_box);

  {
    absl::MutexLock lock(&mutex_);
    scope_tree_.Insert(timer_chain->Last());
  }

  ++num_timers_;
  if (timer_info.start() < min_time_) min_time_ = timer_info.start();
  if (timer_info.end() > max_time_) max_time_ = timer_info.end();
}

float ThreadTrack::GetYFromDepth(uint32_t depth) const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  return pos_[1] - GetHeaderHeight() - layout.GetSpaceBetweenTracksAndThread() -
         box_height_ * static_cast<float>(depth + 1);
}

void ThreadTrack::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                   PickingMode picking_mode, float z_offset) {
  UpdatePositionOfSubtracks();

  if (!thread_state_track_->IsEmpty()) {
    thread_state_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
  if (!event_track_->IsEmpty()) {
    event_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }
  if (!tracepoint_track_->IsEmpty()) {
    tracepoint_track_->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
  }

  UpdateBoxHeight();

  GlCanvas* canvas = time_graph_->GetCanvas();

  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();
  double inv_time_window = 1.0 / time_graph_->GetTimeWindowUs();
  bool is_collapsed = collapse_toggle_->IsCollapsed();

  std::vector<std::shared_ptr<TimerChain>> chains_by_depth = GetTimers();
  const TextBox* selected_textbox = app_->selected_text_box();
  uint64_t highlighted_function_id = app_->GetFunctionIdToHighlight();

  // We minimize overdraw when drawing lines for small events by discarding
  // events that would just draw over an already drawn line. When zoomed in
  // enough that all events are drawn as boxes, this has no effect. When zoomed
  // out, many events will be discarded quickly.
  uint64_t time_window_ns = static_cast<uint64_t>(1000 * time_graph_->GetTimeWindowUs());
  uint64_t pixel_delta_in_ticks = time_window_ns / canvas->GetWidth();
  uint64_t min_timegraph_tick = time_graph_->GetTickFromUs(time_graph_->GetMinTimeUs());

  absl::MutexLock lock(&mutex_);

  if (render_with_nodes_) {
    ORBIT_SCOPE("Render with nodes");
    for (auto& [depth, ordered_nodes] : scope_tree_.GetOrderedNodesByDepth()) {
      // We have to reset this when we go to the next depth, as otherwise we
      // would miss drawing events that should be drawn.
      uint64_t min_ignore = std::numeric_limits<uint64_t>::max();
      uint64_t max_ignore = std::numeric_limits<uint64_t>::min();
      auto first_node_to_draw = ordered_nodes.lower_bound(min_tick);
      if (first_node_to_draw != ordered_nodes.begin()) --first_node_to_draw;
      float world_timer_y = GetYFromDepth(depth - 1);

      for (auto it = first_node_to_draw; it != ordered_nodes.end() && it->first < max_tick; ++it) {
        ScopeNode<TextBox>* node = it->second;
        TextBox& text_box = *node->GetScope();

        const TimerInfo& timer_info = text_box.GetTimerInfo();
        // if (min_tick > timer_info.end() || max_tick < timer_info.start()) continue;
        if (timer_info.start() >= min_ignore && timer_info.end() <= max_ignore) continue;
        // if (!TimerFilter(timer_info)) continue;
        uint64_t function_id = timer_info.function_id();

        UpdateDepth(depth);
        double start_us = time_graph_->GetUsFromTick(timer_info.start());
        double end_us = time_graph_->GetUsFromTick(timer_info.end());
        double elapsed_us = end_us - start_us;
        double normalized_start = start_us * inv_time_window;
        double normalized_length = elapsed_us * inv_time_window;
        float world_timer_width = static_cast<float>(normalized_length * world_width);
        float world_timer_x = static_cast<float>(world_start_x + normalized_start * world_width);

        bool is_visible_width = normalized_length * canvas->GetWidth() > 1;
        bool is_selected = &text_box == selected_textbox;
        bool is_highlighted = !is_selected &&
                              function_id != orbit_grpc_protos::kInvalidFunctionId &&
                              function_id == highlighted_function_id;

        Vec2 pos(world_timer_x, world_timer_y);
        Vec2 size(world_timer_width, GetTextBoxHeight(timer_info));
        float z = GlCanvas::kZValueBox + z_offset;
        const Color kHighlightColor(100, 181, 246, 255);
        Color color = GetTimerColor(timer_info, is_selected, is_highlighted);
        text_box.SetPos(pos);
        text_box.SetSize(size);

        auto user_data = std::make_unique<PickingUserData>(
            &text_box, [&](PickingId id) { return this->GetBoxTooltip(*batcher, id); });

        if (is_visible_width) {
          if (!is_collapsed) {
            SetTimesliceText(timer_info, elapsed_us, world_start_x, z_offset, &text_box);
          }
          batcher->AddShadedBox(pos, size, z, color, std::move(user_data));
        } else {
          batcher->AddVerticalLine(pos, size[1], z, color, std::move(user_data));
          // For lines, we can ignore the entire pixel into which this event
          // falls. We align this precisely on the pixel x-coordinate of the
          // current line being drawn (in ticks). If pixel_delta_in_ticks is
          // zero, we need to avoid dividing by zero, but we also wouldn't
          // gain anything here.
          if (pixel_delta_in_ticks != 0) {
            min_ignore = min_timegraph_tick +
                         ((timer_info.start() - min_timegraph_tick) / pixel_delta_in_ticks) *
                             pixel_delta_in_ticks;
            max_ignore = min_ignore + pixel_delta_in_ticks;
          }
        }
      }
    }

  } else {
    ORBIT_SCOPE("Render with chains");
    for (auto& chain : chains_by_depth) {
      if (!chain) continue;
      for (auto& block : *chain) {
        if (!block.Intersects(min_tick, max_tick)) continue;

        // We have to reset this when we go to the next depth, as otherwise we
        // would miss drawing events that should be drawn.
        uint64_t min_ignore = std::numeric_limits<uint64_t>::max();
        uint64_t max_ignore = std::numeric_limits<uint64_t>::min();

        for (size_t k = 0; k < block.size(); ++k) {
          TextBox& text_box = block[k];
          const TimerInfo& timer_info = text_box.GetTimerInfo();
          if (min_tick > timer_info.end() || max_tick < timer_info.start()) continue;
          if (timer_info.start() >= min_ignore && timer_info.end() <= max_ignore) continue;
          if (!TimerFilter(timer_info)) continue;
          uint64_t function_id = timer_info.function_id();

          UpdateDepth(timer_info.depth() + 1);
          double start_us = time_graph_->GetUsFromTick(timer_info.start());
          double end_us = time_graph_->GetUsFromTick(timer_info.end());
          double elapsed_us = end_us - start_us;
          double normalized_start = start_us * inv_time_window;
          double normalized_length = elapsed_us * inv_time_window;
          float world_timer_width = static_cast<float>(normalized_length * world_width);
          float world_timer_x = static_cast<float>(world_start_x + normalized_start * world_width);
          float world_timer_y = GetYFromTimer(timer_info);

          bool is_visible_width = normalized_length * canvas->GetWidth() > 1;
          bool is_selected = &text_box == selected_textbox;
          bool is_highlighted = !is_selected &&
                                function_id != orbit_grpc_protos::kInvalidFunctionId &&
                                function_id == highlighted_function_id;

          Vec2 pos(world_timer_x, world_timer_y);
          Vec2 size(world_timer_width, GetTextBoxHeight(timer_info));
          float z = GlCanvas::kZValueBox + z_offset;
          const Color kHighlightColor(100, 181, 246, 255);
          Color color = GetTimerColor(timer_info, is_selected, is_highlighted);
          text_box.SetPos(pos);
          text_box.SetSize(size);

          auto user_data = std::make_unique<PickingUserData>(
              &text_box, [&](PickingId id) { return this->GetBoxTooltip(*batcher, id); });

          if (is_visible_width) {
            if (!is_collapsed) {
              SetTimesliceText(timer_info, elapsed_us, world_start_x, z_offset, &text_box);
            }
            batcher->AddShadedBox(pos, size, z, color, std::move(user_data));
          } else {
            batcher->AddVerticalLine(pos, size[1], z, color, std::move(user_data));
            // For lines, we can ignore the entire pixel into which this event
            // falls. We align this precisely on the pixel x-coordinate of the
            // current line being drawn (in ticks). If pixel_delta_in_ticks is
            // zero, we need to avoid dividing by zero, but we also wouldn't
            // gain anything here.
            if (pixel_delta_in_ticks != 0) {
              min_ignore = min_timegraph_tick +
                           ((timer_info.start() - min_timegraph_tick) / pixel_delta_in_ticks) *
                               pixel_delta_in_ticks;
              max_ignore = min_ignore + pixel_delta_in_ticks;
            }
          }
        }
      }
    }
  }
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
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  bool is_manual = timer_info.type() == TimerInfo::kApiEvent;
  if (is_manual) {
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
                                                              const FunctionInfo&) {
  if (timer_info.type() != TimerInfo::kApiEvent) {
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
  std::optional<Color> user_color = GetUserColor(timer_info, *function_info);

  Color color = kInactiveColor;
  if (user_color.has_value()) {
    color = user_color.value();
  } else if (timer_info.type() == TimerInfo::kIntrospection) {
    orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
    color = event.color == kOrbitColorAuto ? time_graph_->GetColor(event.name)
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
