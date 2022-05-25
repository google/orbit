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
#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientProtos/capture_data.pb.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "PickingManager.h"
#include "PrimitiveAssembler.h"
#include "ShortenStringWithEllipsis.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

using orbit_client_data::CallstackData;
using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;
using orbit_client_data::ThreadID;

namespace orbit_gl {

CallstackThreadBar::CallstackThreadBar(CaptureViewElement* parent, OrbitApp* app,
                                       const orbit_gl::TimelineInfoInterface* timeline_info,
                                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                       const orbit_client_data::ModuleManager* module_manager,
                                       const CaptureData* capture_data, ThreadID thread_id,
                                       const Color& color)
    : ThreadBar(parent, app, timeline_info, viewport, layout, module_manager, capture_data,
                thread_id, "Callstacks", color) {}

std::string CallstackThreadBar::GetTooltip() const {
  return "Left-click and drag to select samples";
}

void CallstackThreadBar::DoDraw(PrimitiveAssembler& primitive_assembler,
                                TextRenderer& text_renderer, const DrawContext& draw_context) {
  ThreadBar::DoDraw(primitive_assembler, text_renderer, draw_context);

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
  Quad box = MakeBox(pos, Vec2(GetWidth(), GetHeight()));
  primitive_assembler.AddBox(box, event_bar_z, color, shared_from_this());

  if (primitive_assembler.GetPickingManager()->IsThisElementPicked(this)) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = pos[0];
  float y0 = pos[1];
  float x1 = x0 + GetWidth();
  float y1 = y0 + GetHeight();

  primitive_assembler.AddLine(pos, Vec2(x1, y0), event_bar_z, color, shared_from_this());
  primitive_assembler.AddLine(Vec2(x1, y1), Vec2(x0, y1), event_bar_z, color, shared_from_this());

  if (picked_) {
    Vec2& from = mouse_pos_last_click_;
    Vec2& to = mouse_pos_cur_;

    x0 = from[0];
    y0 = pos[1];
    x1 = to[0];
    y1 = y0 + GetHeight();

    Color picked_color(0, 128, 255, 128);
    Quad picked_box = MakeBox(Vec2(x0, y0), Vec2(x1 - x0, GetHeight()));
    primitive_assembler.AddBox(picked_box, GlCanvas::kZValueUi, picked_color, shared_from_this());
  }
}

void CallstackThreadBar::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                            TextRenderer& text_renderer, uint64_t min_tick,
                                            uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("CallstackThreadBar::DoUpdatePrimitives", kOrbitColorLightBlue);
  ThreadBar::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                picking_mode);

  float z = GlCanvas::kZValueEvent;
  float track_height = layout_->GetEventTrackHeightFromTid(GetThreadId());
  const bool picking = picking_mode != PickingMode::kNone;

  const Color kWhite(255, 255, 255, 255);
  const Color kGreenSelection(0, 255, 0, 255);
  const Color kGreyError(160, 160, 160, 255);
  ORBIT_CHECK(capture_data_ != nullptr);

  if (!picking) {
    // Draw all callstack samples.
    auto action_on_callstack_events = [&](const CallstackEvent& event) {
      const uint64_t time = event.timestamp_ns();
      ORBIT_CHECK(time >= min_tick && time <= max_tick);
      Vec2 pos(timeline_info_->GetWorldFromTick(time), GetPos()[1]);
      Color color = kWhite;
      if (capture_data_->GetCallstackData().GetCallstack(event.callstack_id())->type() !=
          CallstackType::kComplete) {
        color = kGreyError;
      }
      primitive_assembler.AddVerticalLine(pos, track_height, z, color);
    };

    if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
      capture_data_->GetCallstackData().ForEachCallstackEventInTimeRange(
          min_tick, max_tick, action_on_callstack_events);
    } else {
      capture_data_->GetCallstackData().ForEachCallstackEventOfTidInTimeRange(
          GetThreadId(), min_tick, max_tick, action_on_callstack_events);
    }

    // Draw selected callstack samples.
    auto action_on_selected_callstack_events = [&](const CallstackEvent& event) {
      const uint64_t time = event.timestamp_ns();
      ORBIT_CHECK(time >= min_tick && time <= max_tick);
      Vec2 pos(timeline_info_->GetWorldFromTick(event.timestamp_ns()), GetPos()[1]);
      primitive_assembler.AddVerticalLine(pos, track_height, z, kGreenSelection);
    };
    const orbit_client_data::CallstackData& selection_callstack_data =
        capture_data_->selection_callstack_data();
    if (GetThreadId() == orbit_base::kAllProcessThreadsTid) {
      selection_callstack_data.ForEachCallstackEventInTimeRange(
          min_tick, max_tick, action_on_selected_callstack_events);
    } else {
      selection_callstack_data.ForEachCallstackEventOfTidInTimeRange(
          GetThreadId(), min_tick, max_tick, action_on_selected_callstack_events);
    }
  } else {
    // Draw boxes instead of lines to make picking easier, even if this may
    // cause samples to overlap
    constexpr const float kPickingBoxWidth = 9.0f;
    constexpr const float kPickingBoxOffset = (kPickingBoxWidth - 1.0f) / 2.0f;

    auto action_on_callstack_events = [&, this](const CallstackEvent& event) {
      const uint64_t time = event.timestamp_ns();
      ORBIT_CHECK(time >= min_tick && time <= max_tick);
      Vec2 pos(timeline_info_->GetWorldFromTick(time) - kPickingBoxOffset, GetPos()[1]);
      Vec2 size(kPickingBoxWidth, track_height);
      auto user_data = std::make_unique<PickingUserData>(
          nullptr, [this, &primitive_assembler](PickingId id) -> std::string {
            return GetSampleTooltip(primitive_assembler, id);
          });
      user_data->custom_data_ = &event;
      primitive_assembler.AddShadedBox(pos, size, z, kGreenSelection, std::move(user_data));
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

  ORBIT_CHECK(capture_data_);
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

CallstackThreadBar::UnformattedModuleAndFunctionName
CallstackThreadBar::SafeGetModuleAndFunctionName(const CallstackInfo& callstack,
                                                 size_t frame_index) const {
  ORBIT_CHECK(capture_data_ != nullptr);
  if (frame_index >= callstack.frames().size()) {
    static const UnformattedModuleAndFunctionName kFrameIndexTooLargeModuleAndFunctionName = [] {
      UnformattedModuleAndFunctionName module_and_function_name;
      module_and_function_name.module_name = orbit_client_data::kUnknownFunctionOrModuleName;
      module_and_function_name.module_is_unknown = true;
      module_and_function_name.function_name = orbit_client_data::kUnknownFunctionOrModuleName;
      module_and_function_name.function_is_unknown = true;
      return module_and_function_name;
    }();
    return kFrameIndexTooLargeModuleAndFunctionName;
  }

  const uint64_t address = callstack.frames()[frame_index];

  const auto& [module_path, unused_module_build_id] =
      orbit_client_data::FindModulePathAndBuildIdByAddress(*module_manager_, *capture_data_,
                                                           address);
  const std::string unknown_module_path_or_module_name =
      (module_path == orbit_client_data::kUnknownFunctionOrModuleName)
          ? module_path
          : std::filesystem::path(module_path).filename().string();

  const std::string& function_name =
      orbit_client_data::GetFunctionNameByAddress(*module_manager_, *capture_data_, address);
  const std::string function_name_or_unknown_with_address =
      (function_name == orbit_client_data::kUnknownFunctionOrModuleName)
          ? absl::StrFormat("[unknown@%#x]", address)
          : function_name;

  UnformattedModuleAndFunctionName module_and_function_name;
  module_and_function_name.module_name = unknown_module_path_or_module_name;
  module_and_function_name.module_is_unknown =
      (module_path == orbit_client_data::kUnknownFunctionOrModuleName);
  module_and_function_name.function_name = function_name_or_unknown_with_address;
  module_and_function_name.function_is_unknown =
      (function_name == orbit_client_data::kUnknownFunctionOrModuleName);
  return module_and_function_name;
}

std::string CallstackThreadBar::FormatModuleName(
    const CallstackThreadBar::UnformattedModuleAndFunctionName& module_and_function_name) {
  return module_and_function_name.module_is_unknown
             ? absl::StrFormat("<i>%s</i>", module_and_function_name.module_name)
             : module_and_function_name.module_name;
}

std::string CallstackThreadBar::FormatFunctionName(
    const CallstackThreadBar::UnformattedModuleAndFunctionName& module_and_function_name,
    int max_length) {
  const std::string& function_name = module_and_function_name.function_name;
  const std::string shortened_function_name =
      (max_length > 0) ? ShortenStringWithEllipsis(function_name, static_cast<size_t>(max_length))
                       : function_name;
  // Simple HTML escaping.
  const std::string escaped_function_name =
      absl::StrReplaceAll(shortened_function_name, {{"&", "&amp;"}, {"<", "&lt;"}, {">", "&gt;"}});
  return module_and_function_name.function_is_unknown
             ? absl::StrFormat("<i>%s</i>", escaped_function_name)
             : escaped_function_name;
}

constexpr const char* kUnwindErrorColorString = "#ffb000";

std::string CallstackThreadBar::FormatCallstackForTooltip(const CallstackInfo& callstack) const {
  constexpr int kMaxLineLength = 120;
  // If the callstack has more than `kMaxLines` frames, we only show the `kBottomLineCount`
  // outermost frames and the `kMaxLines - kBottomLineCount` innermost frames, separated by
  // `kShortenedForReadabilityString`.
  constexpr int kMaxLines = 20;
  constexpr int kBottomLineCount = 5;
  static_assert(kBottomLineCount < kMaxLines);

  int callstack_size = static_cast<int>(callstack.frames().size());
  const int bottom_n = std::min(kBottomLineCount, callstack_size);
  const int top_n = std::min(kMaxLines, callstack_size) - bottom_n;

  constexpr int kShortenedForReadabilityFakeIndex = -1;
  constexpr const char* kShortenedForReadabilityString = "<i>... shortened for readability ...</i>";
  std::vector<int> frame_indices_to_display;
  for (int i = 0; i < top_n; ++i) {
    frame_indices_to_display.push_back(i);
  }
  if (callstack_size > kMaxLines) {
    frame_indices_to_display.push_back(kShortenedForReadabilityFakeIndex);
  }
  for (int i = callstack_size - bottom_n; i < callstack_size; ++i) {
    frame_indices_to_display.push_back(i);
  }

  std::string result;
  for (int frame_index : frame_indices_to_display) {
    if (frame_index == kShortenedForReadabilityFakeIndex) {
      result.append(kShortenedForReadabilityString).append("<br/>");
      continue;
    }

    static const std::string kModuleFunctionSeparator{" | "};
    const UnformattedModuleAndFunctionName module_and_function_name =
        SafeGetModuleAndFunctionName(callstack, frame_index);
    const std::string formatted_module_name = FormatModuleName(module_and_function_name);
    const std::string formatted_function_name = FormatFunctionName(
        module_and_function_name, kMaxLineLength - module_and_function_name.module_name.size() -
                                      kModuleFunctionSeparator.size());
    const std::string formatted_module_and_function_name =
        formatted_module_name + kModuleFunctionSeparator + formatted_function_name;
    // The first frame is always correct.
    if (callstack.IsUnwindingError() && frame_index > 0) {
      result += absl::StrFormat("<span style=\"color:%s;\">%s</span><br/>", kUnwindErrorColorString,
                                formatted_module_and_function_name);
    } else {
      result.append(formatted_module_and_function_name).append("<br/>");
    }
  }

  return result;
}

std::string CallstackThreadBar::GetSampleTooltip(const PrimitiveAssembler& primitive_assembler,
                                                 PickingId id) const {
  static const std::string unknown_return_text = "Function call information missing";

  const PickingUserData* user_data = primitive_assembler.GetUserData(id);
  if (user_data == nullptr || user_data->custom_data_ == nullptr) {
    return unknown_return_text;
  }

  ORBIT_CHECK(capture_data_ != nullptr);
  const CallstackData& callstack_data = capture_data_->GetCallstackData();
  const auto* callstack_event = static_cast<const CallstackEvent*>(user_data->custom_data_);

  uint64_t callstack_id = callstack_event->callstack_id();
  const CallstackInfo* callstack = callstack_data.GetCallstack(callstack_id);
  if (callstack == nullptr) {
    return unknown_return_text;
  }

  UnformattedModuleAndFunctionName innermost_module_and_function_name =
      SafeGetModuleAndFunctionName(*callstack, 0);
  std::string innermost_formatted_function_name =
      FormatFunctionName(innermost_module_and_function_name, -1);
  std::string innermost_formatted_module_name =
      FormatModuleName(innermost_module_and_function_name);

  std::string result;
  result.append("<p style=\"white-space:pre;\">");  // Prevent word wrapping.
  result += absl::StrFormat("<b>%s</b><br/>", innermost_formatted_function_name);
  result += "<i>Stack sample</i><br/>";
  result += "<br/>";
  result += absl::StrFormat("<b>Module: </b>%s<br/>", innermost_formatted_module_name);
  result += "<br/>";
  if (callstack->IsUnwindingError()) {
    result += absl::StrFormat("<span style=\"color:%s;\">", kUnwindErrorColorString);
    result += "<b>Unwinding error:</b> the stack could not be unwound successfully.<br/>";
    result += orbit_client_data::CallstackTypeToDescription(callstack->type());
    result += "</span><br/>";
    result += "<br/>";
  }
  result += "<b>Callstack:</b><br/>";
  result += FormatCallstackForTooltip(*callstack);
  result += "<br/>";
  result += "<i>To select samples, click the bar & drag across multiple samples</i>";
  result.append("</p>");
  return result;
}

}  // namespace orbit_gl
