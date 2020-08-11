// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EventTrack.h"

#include "Capture.h"
#include "EventTracer.h"
#include "GlCanvas.h"
#include "PickingManager.h"
#include "SamplingProfiler.h"

using orbit_client_protos::CallstackEvent;

EventTrack::EventTrack(TimeGraph* a_TimeGraph) : Track(a_TimeGraph) {
  m_MousePos[0] = m_MousePos[1] = Vec2(0, 0);
  m_Picked = false;
  m_Color = Color(0, 255, 0, 255);
}

std::string EventTrack::GetTooltip() const {
  return "Left-click and drag to select samples";
}

void EventTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  Batcher* batcher = canvas->GetBatcher();
  PickingManager& picking_manager = canvas->GetPickingManager();

  const bool picking = picking_mode != PickingMode::kNone;
  // The sample indicators are at z == 0 and do not respond to clicks, but
  // have a tooltip. For picking, we want to draw the event bar over them if
  // handling a click, and underneath otherwise.
  // This simulates "click-through" behavior.
  const float eventBarZ = picking_mode == PickingMode::kClick
                              ? GlCanvas::Z_VALUE_EVENT_BAR_PICKING
                              : GlCanvas::Z_VALUE_EVENT_BAR;
  Color color = m_Color;

  if (picking) {
    color = picking_manager.GetPickableColor(shared_from_this(),
                                             PickingID::BatcherId::UI);
  }

  Box box(m_Pos, Vec2(m_Size[0], -m_Size[1]), eventBarZ);
  batcher->AddBox(box, color, PickingID::PICKABLE);

  if (canvas->GetPickingManager().IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = m_Pos[0];
  float y0 = m_Pos[1];
  float x1 = x0 + m_Size[0];
  float y1 = y0 - m_Size[1];

  batcher->AddLine(m_Pos, Vec2(x1, y0), GlCanvas::Z_VALUE_EVENT_BAR, color,
                   PickingID::PICKABLE);
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), GlCanvas::Z_VALUE_EVENT_BAR,
                   color, PickingID::PICKABLE);

  if (m_Picked) {
    Vec2& from = m_MousePos[0];
    Vec2& to = m_MousePos[1];

    x0 = from[0];
    y0 = m_Pos[1];
    x1 = to[0];
    y1 = y0 - m_Size[1];

    Color picked_color(0, 128, 255, 128);
    Box box(Vec2(x0, y0), Vec2(x1 - x0, -m_Size[1]), -0.f);
    batcher->AddBox(box, picked_color, PickingID::PICKABLE);
  }

  m_Canvas = canvas;
}

void EventTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick,
                                  PickingMode picking_mode) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float z = GlCanvas::Z_VALUE_EVENT;
  float track_height = layout.GetEventTrackHeight();
  const bool picking = picking_mode != PickingMode::kNone;

  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
  std::map<uint64_t, CallstackEvent>& callstacks =
      GEventTracer.GetEventBuffer().GetCallstacks()[m_ThreadId];

  const Color kWhite(255, 255, 255, 255);
  const Color kGreenSelection(0, 255, 0, 255);

  if (!picking) {
    // Sampling Events
    for (auto& pair : callstacks) {
      uint64_t time = pair.first;
      if (time > min_tick && time < max_tick) {
        Vec2 pos(time_graph_->GetWorldFromTick(time), m_Pos[1]);
        batcher->AddVerticalLine(pos, -track_height, z, kWhite,
                                 PickingID::LINE);
      }
    }

    // Draw selected events
    Color selectedColor[2];
    Fill(selectedColor, kGreenSelection);
    for (const CallstackEvent& event :
         time_graph_->GetSelectedCallstackEvents(m_ThreadId)) {
      Vec2 pos(time_graph_->GetWorldFromTick(event.time()), m_Pos[1]);
      batcher->AddVerticalLine(pos, -track_height, z, kGreenSelection,
                               PickingID::LINE);
    }
  } else {
    // Draw boxes instead of lines to make picking easier, even if this may
    // cause samples to overlap
    constexpr const float kPickingBoxWidth = 9.0f;
    constexpr const float kPickingBoxOffset = (kPickingBoxWidth - 1.0f) / 2.0f;

    for (auto& pair : callstacks) {
      uint64_t time = pair.first;
      if (time > min_tick && time < max_tick) {
        Vec2 pos(time_graph_->GetWorldFromTick(time) - kPickingBoxOffset,
                 m_Pos[1] - track_height + 1);
        Vec2 size(kPickingBoxWidth, track_height);
        auto user_data = std::make_unique<PickingUserData>(
            nullptr,
            [&](PickingID id) -> std::string { return GetSampleTooltip(id); });
        user_data->custom_data_ = &pair.second;
        batcher->AddShadedBox(pos, size, z, kGreenSelection, PickingID::BOX,
                              std::move(user_data));
      }
    }
  }
}

void EventTrack::SetPos(float a_X, float a_Y) {
  m_Pos = Vec2(a_X, a_Y);
  m_ThreadName.SetPos(Vec2(a_X, a_Y));
  m_ThreadName.SetSize(Vec2(m_Size[0] * 0.3f, m_Size[1]));
}

void EventTrack::SetSize(float a_SizeX, float a_SizeY) {
  m_Size = Vec2(a_SizeX, a_SizeY);
}

void EventTrack::OnPick(int a_X, int a_Y) {
  Capture::GSelectedThreadId = m_ThreadId;
  Vec2& mousePos = m_MousePos[0];
  m_Canvas->ScreenToWorld(a_X, a_Y, mousePos[0], mousePos[1]);
  m_MousePos[1] = m_MousePos[0];
  m_Picked = true;
}

void EventTrack::OnRelease() {
  if (m_Picked) {
    SelectEvents();
  }

  m_Picked = false;
}

void EventTrack::OnDrag(int a_X, int a_Y) {
  Vec2& to = m_MousePos[1];
  m_Canvas->ScreenToWorld(a_X, a_Y, to[0], to[1]);
}

void EventTrack::SelectEvents() {
  Vec2& from = m_MousePos[0];
  Vec2& to = m_MousePos[1];

  time_graph_->SelectEvents(from[0], to[0], m_ThreadId);
}

bool EventTrack::IsEmpty() const {
  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
  const std::map<uint64_t, CallstackEvent>& callstacks =
      GEventTracer.GetEventBuffer().GetCallstacks()[m_ThreadId];
  return callstacks.empty();
}

static std::string SafeGetFormattedFunctionName(uint64_t addr,
                                                int max_line_length) {
  const std::string& function_name =
      Capture::GSamplingProfiler->GetFunctionNameByAddress(addr);
  if (function_name == SamplingProfiler::kUnknownFunctionOrModuleName) {
    return std::string("<i>") + function_name + "</i>";
  }

  std::string fn_name =
      max_line_length >= 0
          ? ShortenStringWithEllipsis(function_name,
                                      static_cast<size_t>(max_line_length))
          : function_name;
  // Simple HTML escaping
  fn_name = Replace(fn_name, "&", "&amp;");
  fn_name = Replace(fn_name, "<", "&lt;");
  fn_name = Replace(fn_name, ">", "&gt;");
  return fn_name;
};

static std::string FormatCallstackForTooltip(
    std::shared_ptr<CallStack> callstack, int max_line_length = 80,
    int max_lines = 20, int bottom_n_lines = 5) {
  std::string result;
  int size = static_cast<int>(callstack->GetFramesCount());
  if (max_lines <= 0) {
    max_lines = size;
  }
  const int bottom_n = std::min(std::min(max_lines - 1, bottom_n_lines), size);
  const int top_n = std::min(max_lines, size) - bottom_n;

  for (int i = 0; i < top_n; ++i) {
    result =
        result + "<br/>" +
        SafeGetFormattedFunctionName(callstack->GetFrame(i), max_line_length);
  }
  if (max_lines < size) {
    result += "<br/><i>... shortened for readability ...</i>";
  }
  for (int i = size - bottom_n; i < size; ++i) {
    result =
        result + "<br/>" +
        SafeGetFormattedFunctionName(callstack->GetFrame(i), max_line_length);
  }

  return result;
}

std::string EventTrack::GetSampleTooltip(PickingID id) const {
  static const std::string unknown_return_text =
      "Function call information missing";

  auto user_data = time_graph_->GetBatcher().GetUserData(id);
  if (!user_data || !user_data->custom_data_) {
    return unknown_return_text;
  }

  CallstackEvent* callstack_event =
      static_cast<CallstackEvent*>(user_data->custom_data_);
  auto callstack = Capture::GSamplingProfiler->GetCallStack(
      callstack_event->callstack_hash());

  if (!callstack) {
    return unknown_return_text;
  }

  std::string function_name =
      SafeGetFormattedFunctionName(callstack->GetFrame(0), -1);
  std::string result = absl::StrFormat(
      "<b>%s</b><br/><i>Sampled event</i><br/><br/><b>Callstack:</b>",
      function_name.c_str());
  result += FormatCallstackForTooltip(callstack);
  return result +
         "<br/><br/><i>To select samples, click the bar & drag across multiple "
         "samples</i>";
}
