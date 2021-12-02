// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallstackThreadBar.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <stddef.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "App.h"
#include "Batcher.h"
#include "ClientData/CallstackData.h"
#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "PickingManager.h"
#include "ShortenStringWithEllipsis.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

using orbit_client_data::CallstackData;
using orbit_client_data::CaptureData;
using orbit_client_data::ThreadID;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;

namespace orbit_gl {

CallstackThreadBar::CallstackThreadBar(CaptureViewElement* parent, OrbitApp* app,
                                       const orbit_gl::TimelineInfoInterface* timeline_info,
                                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                       const CaptureData* capture_data, ThreadID thread_id,
                                       const Color& color)
    : ThreadBar(parent, app, timeline_info, viewport, layout, capture_data, thread_id, "Callstacks",
                color) {}

std::string CallstackThreadBar::GetTooltip() const {
  return "Left-click and drag to select samples";
}

void CallstackThreadBar::DoDraw(Batcher& batcher, TextRenderer& text_renderer,
                                const DrawContext& draw_context) {
  ThreadBar::DoDraw(batcher, text_renderer, draw_context);

  if (GetThreadId() == orbit_base::kAllThreadsOfAllProcessesTid) {
    return;
  }

  // The sample indicators are at z == 0 and do not respond to clicks, but
  // have a tooltip. For picking, we want to draw the event bar over them if
  // handling a click, and underneath otherwise.
  // This simulates "click-through" behavior.
  float event_bar_z = draw_context.picking_mode == PickingMode::kClick
                          ? GlCanvas::kZValueEventBarPicking
                          : GlCanvas::kZValueEventBar;
  Color color = GetColor();
  const Vec2 pos = GetPos();
  Box box(pos, Vec2(GetWidth(), GetHeight()), event_bar_z);
  batcher.AddBox(box, color, shared_from_this());

  if (batcher.GetPickingManager()->IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = pos[0];
  float y0 = pos[1];
  float x1 = x0 + GetWidth();
  float y1 = y0 + GetHeight();

  batcher.AddLine(pos, Vec2(x1, y0), event_bar_z, color, shared_from_this());
  batcher.AddLine(Vec2(x1, y1), Vec2(x0, y1), event_bar_z, color, shared_from_this());

  if (picked_) {
    Vec2& from = mouse_pos_last_click_;
    Vec2& to = mouse_pos_cur_;

    x0 = from[0];
    y0 = pos[1];
    x1 = to[0];
    y1 = y0 + GetHeight();

    Color picked_color(0, 128, 255, 128);
    Box picked_box(Vec2(x0, y0), Vec2(x1 - x0, GetHeight()), GlCanvas::kZValueUi);
    batcher.AddBox(picked_box, picked_color, shared_from_this());
  }
}

void CallstackThreadBar::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                            uint64_t min_tick, uint64_t max_tick,
                                            PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("CallstackThreadBar::DoUpdatePrimitives", kOrbitColorLightBlue);
  ThreadBar::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);

  float z = GlCanvas::kZValueEvent;
  float track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const bool picking = picking_mode != PickingMode::kNone;

  const Color kWhite(255, 255, 255, 255);
  const Color kGreenSelection(0, 255, 0, 255);
  const Color kGreyError(160, 160, 160, 255);
  CHECK(capture_data_ != nullptr);

  if (!picking) {
    // Sampling Events
    auto action_on_callstack_events = [&](const CallstackEvent& event) {
      const uint64_t time = event.time();
      CHECK(time >= min_tick && time <= max_tick);
      Vec2 pos(timeline_info_->GetWorldFromTick(time), GetPos()[1]);
      Color color = kWhite;
      if (capture_data_->GetCallstackData().GetCallstack(event.callstack_id())->type() !=
          CallstackInfo::kComplete) {
        color = kGreyError;
      }
      batcher.AddVerticalLine(pos, track_height, z, color);
    };

    if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
      capture_data_->GetCallstackData().ForEachCallstackEventInTimeRange(
          min_tick, max_tick, action_on_callstack_events);
    } else {
      capture_data_->GetCallstackData().ForEachCallstackEventOfTidInTimeRange(
          GetThreadId(), min_tick, max_tick, action_on_callstack_events);
    }

    // Draw selected events
    std::array<Color, 2> selected_color;
    selected_color.fill(kGreenSelection);
    for (const CallstackEvent& event : app_->GetSelectedCallstackEvents(GetThreadId())) {
      Vec2 pos(timeline_info_->GetWorldFromTick(event.time()), GetPos()[1]);
      batcher.AddVerticalLine(pos, track_height, z, kGreenSelection);
    }
  } else {
    // Draw boxes instead of lines to make picking easier, even if this may
    // cause samples to overlap
    constexpr const float kPickingBoxWidth = 9.0f;
    constexpr const float kPickingBoxOffset = (kPickingBoxWidth - 1.0f) / 2.0f;

    auto action_on_callstack_events = [&, this](const CallstackEvent& event) {
      const uint64_t time = event.time();
      CHECK(time >= min_tick && time <= max_tick);
      Vec2 pos(timeline_info_->GetWorldFromTick(time) - kPickingBoxOffset, GetPos()[1]);
      Vec2 size(kPickingBoxWidth, track_height);
      auto user_data = std::make_unique<PickingUserData>(
          nullptr,
          [this, &batcher](PickingId id) -> std::string { return GetSampleTooltip(batcher, id); });
      user_data->custom_data_ = &event;
      batcher.AddShadedBox(pos, size, z, kGreenSelection, std::move(user_data));
    };
    if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
      capture_data_->GetCallstackData().ForEachCallstackEventInTimeRange(
          min_tick, max_tick, action_on_callstack_events);
    } else {
      capture_data_->GetCallstackData().ForEachCallstackEventOfTidInTimeRange(
          GetThreadId(), min_tick, max_tick, action_on_callstack_events);
    }
  }
}

void CallstackThreadBar::OnRelease() {
  CaptureViewElement::OnRelease();
  SelectCallstacks();
}

void CallstackThreadBar::OnPick(int x, int y) {
  CaptureViewElement::OnPick(x, y);
  app_->set_selected_thread_id(GetThreadId());
}

void CallstackThreadBar::SelectCallstacks() {
  Vec2 from = mouse_pos_last_click_;
  Vec2 to = mouse_pos_cur_;

  if (from > to) {
    std::swap(from, to);
  }

  uint64_t t0 = timeline_info_->GetTickFromWorld(from[0]);
  uint64_t t1 = timeline_info_->GetTickFromWorld(to[0]);

  int64_t thread_id = GetThreadId();
  bool thread_id_is_all_threads = thread_id == orbit_base::kAllProcessThreadsTid;

  CHECK(capture_data_);
  auto selected_callstack_events =
      thread_id_is_all_threads
          ? capture_data_->GetCallstackData().GetCallstackEventsInTimeRange(t0, t1)
          : capture_data_->GetCallstackData().GetCallstackEventsOfTidInTimeRange(thread_id, t0, t1);

  app_->SelectCallstackEvents(selected_callstack_events, thread_id_is_all_threads);
}

bool CallstackThreadBar::IsEmpty() const {
  if (capture_data_ == nullptr) {
    return true;
  }

  const uint32_t callstack_count =
      (GetThreadId() == orbit_base::kAllProcessThreadsTid)
          ? capture_data_->GetCallstackData().GetCallstackEventsCount()
          : capture_data_->GetCallstackData().GetCallstackEventsOfTidCount(GetThreadId());
  return callstack_count == 0;
}

[[nodiscard]] std::string CallstackThreadBar::SafeGetFormattedFunctionName(
    const orbit_client_protos::CallstackInfo& callstack, int frame_index,
    int max_line_length) const {
  CHECK(capture_data_ != nullptr);
  if (frame_index >= callstack.frames_size()) {
    return std::string("<i>") + CaptureData::kUnknownFunctionOrModuleName + "</i>";
  }

  const uint64_t addr = callstack.frames(frame_index);
  const std::string& function_name = capture_data_->GetFunctionNameByAddress(addr);
  if (function_name == CaptureData::kUnknownFunctionOrModuleName) {
    return std::string("<i>") + absl::StrFormat("[unknown@%#x]", addr) + "</i>";
  }

  std::string fn_name =
      max_line_length >= 0
          ? orbit_gl::ShortenStringWithEllipsis(function_name, static_cast<size_t>(max_line_length))
          : function_name;
  // Simple HTML escaping
  return absl::StrReplaceAll(fn_name, {{"&", "&amp;"}, {"<", "&lt;"}, {">", "&gt;"}});
}

std::string CallstackThreadBar::FormatCallstackForTooltip(const CallstackInfo& callstack,
                                                          int max_line_length, int max_lines,
                                                          int bottom_n_lines) const {
  std::string result;
  int size = static_cast<int>(callstack.frames_size());
  if (max_lines <= 0) {
    max_lines = size;
  }
  const int bottom_n = std::min(std::min(max_lines - 1, bottom_n_lines), size);
  const int top_n = std::min(max_lines, size) - bottom_n;

  for (int i = 0; i < top_n; ++i) {
    result.append("<br/>" + SafeGetFormattedFunctionName(callstack, i, max_line_length));
  }
  if (max_lines < size) {
    result += "<br/><i>... shortened for readability ...</i>";
  }
  for (int i = size - bottom_n; i < size; ++i) {
    result.append("<br/>" + SafeGetFormattedFunctionName(callstack, i, max_line_length));
  }

  return result;
}

std::string CallstackThreadBar::GetSampleTooltip(const Batcher& batcher, PickingId id) const {
  static const std::string unknown_return_text = "Function call information missing";

  const PickingUserData* user_data = batcher.GetUserData(id);
  if (user_data == nullptr || user_data->custom_data_ == nullptr) {
    return unknown_return_text;
  }

  CHECK(capture_data_ != nullptr);
  const CallstackData& callstack_data = capture_data_->GetCallstackData();
  const auto* callstack_event = static_cast<const CallstackEvent*>(user_data->custom_data_);

  uint64_t callstack_id = callstack_event->callstack_id();
  const CallstackInfo* callstack = callstack_data.GetCallstack(callstack_id);
  if (callstack == nullptr) {
    return unknown_return_text;
  }

  std::string function_name = SafeGetFormattedFunctionName(*callstack, 0, -1);
  std::string result =
      absl::StrFormat("<b>%s</b><br/><i>Stack sample</i><br/><br/>", function_name.c_str());
  if (callstack->type() == CallstackInfo::kComplete) {
    result += "<b>Callstack:</b>" + FormatCallstackForTooltip(*callstack);
  } else {
    // TODO(b/188756080): Show a specific explanation for each CallstackType.
    result += "Callstack not available: the stack could not be unwound successfully";
    if (app_->IsDevMode()) {
      result += absl::StrFormat(" (%s)", CallstackInfo::CallstackType_Name(callstack->type()));
    }
  }
  return result +
         "<br/><br/><i>To select samples, click the bar & drag across multiple samples</i>";
}

}  // namespace orbit_gl
