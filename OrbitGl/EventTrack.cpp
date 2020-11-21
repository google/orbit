// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EventTrack.h"

#include "App.h"
#include "GlCanvas.h"
#include "PickingManager.h"

using orbit_client_protos::CallstackEvent;

EventTrack::EventTrack(TimeGraph* a_TimeGraph) : Track(a_TimeGraph) {
  mouse_pos_[0] = mouse_pos_[1] = Vec2(0, 0);
  picked_ = false;
  color_ = Color(0, 255, 0, 255);
}

std::string EventTrack::GetTooltip() const { return "Left-click and drag to select samples"; }

void EventTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  if (thread_id_ == TracepointEventBuffer::kAllTracepointsFakeTid) {
    return;
  }

  Batcher* batcher = canvas->GetBatcher();

  // The sample indicators are at z == 0 and do not respond to clicks, but
  // have a tooltip. For picking, we want to draw the event bar over them if
  // handling a click, and underneath otherwise.
  // This simulates "click-through" behavior.
  float event_bar_z = picking_mode == PickingMode::kClick ? GlCanvas::kZValueEventBarPicking
                                                          : GlCanvas::kZValueEventBar;
  event_bar_z += z_offset;
  Color color = color_;
  Box box(pos_, Vec2(size_[0], -size_[1]), event_bar_z);
  batcher->AddBox(box, color, shared_from_this());

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = pos_[0];
  float y0 = pos_[1];
  float x1 = x0 + size_[0];
  float y1 = y0 - size_[1];

  batcher->AddLine(pos_, Vec2(x1, y0), event_bar_z, color, shared_from_this());
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), event_bar_z, color, shared_from_this());

  if (picked_) {
    Vec2& from = mouse_pos_[0];
    Vec2& to = mouse_pos_[1];

    x0 = from[0];
    y0 = pos_[1];
    x1 = to[0];
    y1 = y0 - size_[1];

    Color picked_color(0, 128, 255, 128);
    Box box(Vec2(x0, y0), Vec2(x1 - x0, -size_[1]), GlCanvas::kZValueUi + z_offset);
    batcher->AddBox(box, picked_color, shared_from_this());
  }

  canvas_ = canvas;
}

void EventTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode,
                                  float z_offset) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float z = GlCanvas::kZValueEvent + z_offset;
  float track_height = layout.GetEventTrackHeight();
  const bool picking = picking_mode != PickingMode::kNone;

  const Color kWhite(255, 255, 255, 255);
  const Color kGreenSelection(0, 255, 0, 255);

  if (!picking) {
    // Sampling Events
    auto action_on_callstack_events = [=](const orbit_client_protos::CallstackEvent& event) {
      uint64_t time = event.time();
      if (time > min_tick && time < max_tick) {
        Vec2 pos(time_graph_->GetWorldFromTick(time), pos_[1]);
        batcher->AddVerticalLine(pos, -track_height, z, kWhite);
      }
    };
    if (thread_id_ == SamplingProfiler::kAllThreadsFakeTid) {
      GOrbitApp->GetCaptureData().GetCallstackData()->ForEachCallstackEvent(
          action_on_callstack_events);
    } else {
      GOrbitApp->GetCaptureData().GetCallstackData()->ForEachCallstackEventOfTid(
          thread_id_, action_on_callstack_events);
    }

    // Draw selected events
    Color selectedColor[2];
    Fill(selectedColor, kGreenSelection);
    for (const CallstackEvent& event : time_graph_->GetSelectedCallstackEvents(thread_id_)) {
      Vec2 pos(time_graph_->GetWorldFromTick(event.time()), pos_[1]);
      batcher->AddVerticalLine(pos, -track_height, z, kGreenSelection);
    }
  } else {
    // Draw boxes instead of lines to make picking easier, even if this may
    // cause samples to overlap
    constexpr const float kPickingBoxWidth = 9.0f;
    constexpr const float kPickingBoxOffset = (kPickingBoxWidth - 1.0f) / 2.0f;

    auto action_on_callstack_events = [=](const orbit_client_protos::CallstackEvent& event) {
      uint64_t time = event.time();
      if (time > min_tick && time < max_tick) {
        Vec2 pos(time_graph_->GetWorldFromTick(time) - kPickingBoxOffset,
                 pos_[1] - track_height + 1);
        Vec2 size(kPickingBoxWidth, track_height);
        auto user_data = std::make_unique<PickingUserData>(
            nullptr, [&](PickingId id) -> std::string { return GetSampleTooltip(id); });
        user_data->custom_data_ = &event;
        batcher->AddShadedBox(pos, size, z, kGreenSelection, std::move(user_data));
      }
    };
    if (thread_id_ == SamplingProfiler::kAllThreadsFakeTid) {
      GOrbitApp->GetCaptureData().GetCallstackData()->ForEachCallstackEvent(
          action_on_callstack_events);
    } else {
      GOrbitApp->GetCaptureData().GetCallstackData()->ForEachCallstackEventOfTid(
          thread_id_, action_on_callstack_events);
    }
  }
}

void EventTrack::SetPos(float a_X, float a_Y) { pos_ = Vec2(a_X, a_Y); }

void EventTrack::SetSize(float a_SizeX, float a_SizeY) { size_ = Vec2(a_SizeX, a_SizeY); }

void EventTrack::OnPick(int x, int y) {
  GOrbitApp->set_selected_thread_id(thread_id_);
  Vec2& mouse_pos = mouse_pos_[0];
  canvas_->ScreenToWorld(x, y, mouse_pos[0], mouse_pos[1]);
  mouse_pos_[1] = mouse_pos_[0];
  picked_ = true;
}

void EventTrack::OnRelease() {
  if (picked_) {
    SelectEvents();
  }

  picked_ = false;
}

void EventTrack::OnDrag(int a_X, int a_Y) {
  Vec2& to = mouse_pos_[1];
  canvas_->ScreenToWorld(a_X, a_Y, to[0], to[1]);
}

void EventTrack::SelectEvents() {
  Vec2& from = mouse_pos_[0];
  Vec2& to = mouse_pos_[1];

  time_graph_->SelectEvents(from[0], to[0], thread_id_);
}

bool EventTrack::IsEmpty() const {
  if (!GOrbitApp->HasCaptureData()) return true;
  const uint32_t callstack_count =
      (thread_id_ == SamplingProfiler::kAllThreadsFakeTid)
          ? GOrbitApp->GetCaptureData().GetCallstackData()->GetCallstackEventsCount()
          : GOrbitApp->GetCaptureData().GetCallstackData()->GetCallstackEventsOfTidCount(
                thread_id_);
  return callstack_count == 0;
}

static std::string SafeGetFormattedFunctionName(uint64_t addr, int max_line_length) {
  const std::string& function_name = GOrbitApp->GetCaptureData().GetFunctionNameByAddress(addr);
  if (function_name == CaptureData::kUnknownFunctionOrModuleName) {
    return std::string("<i>") + function_name + "</i>";
  }

  std::string fn_name =
      max_line_length >= 0
          ? ShortenStringWithEllipsis(function_name, static_cast<size_t>(max_line_length))
          : function_name;
  // Simple HTML escaping
  fn_name = Replace(fn_name, "&", "&amp;");
  fn_name = Replace(fn_name, "<", "&lt;");
  fn_name = Replace(fn_name, ">", "&gt;");
  return fn_name;
};

static std::string FormatCallstackForTooltip(const CallStack& callstack, int max_line_length = 80,
                                             int max_lines = 20, int bottom_n_lines = 5) {
  std::string result;
  int size = static_cast<int>(callstack.GetFramesCount());
  if (max_lines <= 0) {
    max_lines = size;
  }
  const int bottom_n = std::min(std::min(max_lines - 1, bottom_n_lines), size);
  const int top_n = std::min(max_lines, size) - bottom_n;

  for (int i = 0; i < top_n; ++i) {
    result =
        result + "<br/>" + SafeGetFormattedFunctionName(callstack.GetFrame(i), max_line_length);
  }
  if (max_lines < size) {
    result += "<br/><i>... shortened for readability ...</i>";
  }
  for (int i = size - bottom_n; i < size; ++i) {
    result =
        result + "<br/>" + SafeGetFormattedFunctionName(callstack.GetFrame(i), max_line_length);
  }

  return result;
}

std::string EventTrack::GetSampleTooltip(PickingId id) const {
  static const std::string unknown_return_text = "Function call information missing";

  auto user_data = time_graph_->GetBatcher().GetUserData(id);
  if (!user_data || !user_data->custom_data_) {
    return unknown_return_text;
  }

  const CallstackData* callstack_data = GOrbitApp->GetCaptureData().GetCallstackData();
  const auto* callstack_event = static_cast<const CallstackEvent*>(user_data->custom_data_);

  uint64_t callstack_hash = callstack_event->callstack_hash();
  const CallStack* callstack = callstack_data->GetCallStack(callstack_hash);
  if (callstack == nullptr) {
    return unknown_return_text;
  }

  std::string function_name = SafeGetFormattedFunctionName(callstack->GetFrame(0), -1);
  std::string result = absl::StrFormat(
      "<b>%s</b><br/><i>Sampled event</i><br/><br/><b>Callstack:</b>", function_name.c_str());
  result += FormatCallstackForTooltip(*callstack);
  return result +
         "<br/><br/><i>To select samples, click the bar & drag across multiple "
         "samples</i>";
}
