// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraph.h"

#include <GteVector.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stddef.h>

#include <algorithm>
#include <limits>

#include "App.h"
#include "AsyncTrack.h"
#include "CGroupAndProcessMemoryTrack.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/FunctionUtils.h"
#include "ClientFlags/ClientFlags.h"
#include "FrameTrack.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "GpuTrack.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"
#include "PageFaultsTrack.h"
#include "PickingManager.h"
#include "SchedulerTrack.h"
#include "SystemMemoryTrack.h"
#include "TrackManager.h"
#include "VariableTrack.h"

using orbit_capture_client::CaptureEventProcessor;

using orbit_client_data::CallstackEvent;
using orbit_client_data::CaptureData;
using orbit_client_data::TimerChain;
using orbit_client_protos::ApiTrackValue;
using orbit_client_protos::TimerInfo;

using orbit_gl::CGroupAndProcessMemoryTrack;
using orbit_gl::PageFaultsTrack;
using orbit_gl::SystemMemoryTrack;
using orbit_gl::TrackManager;
using orbit_gl::VariableTrack;

using orbit_grpc_protos::InstrumentedFunction;

TimeGraph::TimeGraph(AccessibleInterfaceProvider* parent, OrbitApp* app,
                     orbit_gl::Viewport* viewport, CaptureData* capture_data,
                     PickingManager* picking_manager)
    // Note that `GlCanvas` and `TimeGraph` span the bridge to OpenGl content, and `TimeGraph`'s
    // parent needs special handling for accessibility. Thus, we use `nullptr` here and we save the
    // parent in accessible_parent_ which doesn't need to be a CaptureViewElement.
    : orbit_gl::CaptureViewElement(nullptr, viewport, &layout_),
      accessible_parent_{parent},
      batcher_(BatcherId::kTimeGraph),
      manual_instrumentation_manager_{app->GetManualInstrumentationManager()},
      thread_track_data_provider_(capture_data->GetThreadTrackDataProvider()),
      capture_data_{capture_data},
      app_{app} {
  text_renderer_static_.Init();
  text_renderer_static_.SetViewport(viewport);
  batcher_.SetPickingManager(picking_manager);
  track_container_ = std::make_unique<orbit_gl::TrackContainer>(
      this, this, viewport, &layout_, app, app->GetModuleManager(), capture_data);
  timeline_ui_ = std::make_unique<orbit_gl::TimelineUi>(
      /*parent=*/this, /*timeline_info_interface=*/this, viewport, &layout_);
  if (absl::GetFlag(FLAGS_enforce_full_redraw)) {
    RequestUpdate();
  }
}

float TimeGraph::GetHeight() const {
  return viewport_->GetWorldHeight() - layout_.GetBottomMargin();
}

void TimeGraph::UpdateCaptureMinMaxTimestamps() {
  auto [tracks_min_time, tracks_max_time] = GetTrackManager()->GetTracksMinMaxTimestamps();

  capture_min_timestamp_ = std::min(capture_min_timestamp_, tracks_min_time);
  capture_max_timestamp_ = std::max(capture_max_timestamp_, tracks_max_time);
}

void TimeGraph::ZoomAll() {
  constexpr double kNumHistorySeconds = 2.f;
  UpdateCaptureMinMaxTimestamps();
  max_time_us_ = TicksToMicroseconds(capture_min_timestamp_, capture_max_timestamp_);
  min_time_us_ = max_time_us_ - (kNumHistorySeconds * 1000 * 1000);
  if (min_time_us_ < 0) min_time_us_ = 0;

  RequestUpdate();
}

void TimeGraph::Zoom(uint64_t min, uint64_t max) {
  double start = TicksToMicroseconds(capture_min_timestamp_, min);
  double end = TicksToMicroseconds(capture_min_timestamp_, max);

  double mid = start + ((end - start) / 2.0);
  double extent = 1.1 * (end - start) / 2.0;

  SetMinMax(mid - extent, mid + extent);
}

void TimeGraph::Zoom(const TimerInfo& timer_info) { Zoom(timer_info.start(), timer_info.end()); }

double TimeGraph::GetCaptureTimeSpanUs() const {
  // Do we have an empty capture?
  if (capture_max_timestamp_ == 0 &&
      capture_min_timestamp_ == std::numeric_limits<uint64_t>::max()) {
    return 0.0;
  }
  ORBIT_CHECK(capture_min_timestamp_ <= capture_max_timestamp_);
  return TicksToMicroseconds(capture_min_timestamp_, capture_max_timestamp_);
}

double TimeGraph::GetCurrentTimeSpanUs() const { return max_time_us_ - min_time_us_; }

void TimeGraph::ZoomTime(float zoom_value, double mouse_ratio) {
  static double increment_ratio = 0.1;
  double scale = (zoom_value > 0) ? (1 + increment_ratio) : (1 / (1 + increment_ratio));
  // The horizontal zoom could have been triggered from the margin of TimeGraph, so we clamp the
  // mouse_ratio to ensure it is between 0 and 1.
  mouse_ratio = std::clamp(mouse_ratio, 0., 1.);

  double current_time_window_us = max_time_us_ - min_time_us_;
  ref_time_us_ = min_time_us_ + mouse_ratio * current_time_window_us;

  double time_left = std::max(ref_time_us_ - min_time_us_, 0.0);
  double time_right = std::max(max_time_us_ - ref_time_us_, 0.0);

  double min_time_us = ref_time_us_ - scale * time_left;
  double max_time_us = ref_time_us_ + scale * time_right;

  SetMinMax(min_time_us, max_time_us);
}

void TimeGraph::VerticalZoom(float zoom_value, float mouse_world_y_pos) {
  constexpr float kIncrementRatio = 0.1f;
  const float proposed_ratio =
      (zoom_value < 0) ? (1 + kIncrementRatio) : (1 / (1 + kIncrementRatio));

  // We have to scale every item in the layout.
  const float old_scale = layout_.GetScale();
  layout_.SetScale(old_scale * proposed_ratio);

  // As we have maximum/minimum scale, the real ratio might be different than the proposed one.
  const float real_ratio = layout_.GetScale() / old_scale;

  // TODO(b/214270440): Behave differently when the mouse is on top of the timeline.
  track_container_->VerticalZoom(real_ratio, mouse_world_y_pos);
}

// TODO(b/214280802): include SetMinMax in the TimelineInfoInterface, so the scrollbar could call
// it.
void TimeGraph::SetMinMax(double min_time_us, double max_time_us) {
  constexpr double kTimeGraphMinTimeWindowsUs = 0.1; /* 100 ns */
  const double desired_time_window =
      std::max(max_time_us - min_time_us, kTimeGraphMinTimeWindowsUs);

  // Centering the interval in screen.
  const double center_time_us = (max_time_us + min_time_us) / 2.;

  min_time_us_ = std::max(center_time_us - desired_time_window / 2, 0.0);
  max_time_us_ = std::min(min_time_us_ + desired_time_window, GetCaptureTimeSpanUs());

  RequestUpdate();
}

void TimeGraph::PanTime(int initial_x, int current_x, int width, double initial_time) {
  time_window_us_ = max_time_us_ - min_time_us_;
  double initial_local_time = static_cast<double>(initial_x) / width * time_window_us_;
  double dt = static_cast<double>(current_x - initial_x) / width * time_window_us_;
  double current_time = initial_time - dt;
  min_time_us_ =
      std::clamp(current_time - initial_local_time, 0.0, GetCaptureTimeSpanUs() - time_window_us_);
  max_time_us_ = min_time_us_ + time_window_us_;

  RequestUpdate();
}

void TimeGraph::HorizontallyMoveIntoView(VisibilityType vis_type, uint64_t min, uint64_t max,
                                         double distance) {
  if (IsVisible(vis_type, min, max)) {
    return;
  }

  double start = TicksToMicroseconds(capture_min_timestamp_, min);
  double end = TicksToMicroseconds(capture_min_timestamp_, max);

  double current_time_window_us = max_time_us_ - min_time_us_;

  if (vis_type == VisibilityType::kFullyVisible && current_time_window_us < (end - start)) {
    Zoom(min, max);
    return;
  }

  double mid = start + ((end - start) / 2.0);

  // Mirror the final center position if we have to move left
  if (start < min_time_us_) {
    distance = 1 - distance;
  }

  SetMinMax(mid - current_time_window_us * (1 - distance), mid + current_time_window_us * distance);
}

void TimeGraph::HorizontallyMoveIntoView(VisibilityType vis_type, const TimerInfo& timer_info,
                                         double distance) {
  HorizontallyMoveIntoView(vis_type, timer_info.start(), timer_info.end(), distance);
}

void TimeGraph::UpdateHorizontalScroll(float ratio) {
  double time_span = GetCaptureTimeSpanUs();
  double time_window = max_time_us_ - min_time_us_;
  min_time_us_ = ratio * (time_span - time_window);
  max_time_us_ = min_time_us_ + time_window;
}

double TimeGraph::GetTime(double ratio) const {
  double current_width = max_time_us_ - min_time_us_;
  double delta = ratio * current_width;
  return min_time_us_ + delta;
}

void TimeGraph::ProcessTimer(const TimerInfo& timer_info, const InstrumentedFunction* function) {
  capture_min_timestamp_ = std::min(capture_min_timestamp_, timer_info.start());
  capture_max_timestamp_ = std::max(capture_max_timestamp_, timer_info.end());

  TrackManager* track_manager = GetTrackManager();
  // TODO(b/175869409): Change the way to create and get the tracks. Move this part to TrackManager.
  switch (timer_info.type()) {
    // All GPU timers are handled equally here.
    case TimerInfo::kGpuActivity:
    case TimerInfo::kGpuCommandBuffer:
    case TimerInfo::kGpuDebugMarker: {
      uint64_t timeline_hash = timer_info.timeline_hash();
      GpuTrack* track = track_manager->GetOrCreateGpuTrack(timeline_hash);
      track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kFrame: {
      if (function == nullptr) {
        break;
      }
      FrameTrack* track = track_manager->GetOrCreateFrameTrack(*function);
      track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kCoreActivity: {
      // TODO(b/176962090): We need to create the `ThreadTrack` here even we don't use it, as we
      //  don't create it on new callstack events, yet.
      track_manager->GetOrCreateThreadTrack(timer_info.thread_id());
      SchedulerTrack* scheduler_track = track_manager->GetOrCreateSchedulerTrack();
      scheduler_track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kSystemMemoryUsage: {
      ProcessSystemMemoryTrackingTimer(timer_info);
      break;
    }
    case TimerInfo::kCGroupAndProcessMemoryUsage: {
      ProcessCGroupAndProcessMemoryTrackingTimer(timer_info);
      break;
    }
    case TimerInfo::kPageFaults: {
      ProcessPageFaultsTrackingTimer(timer_info);
      break;
    }
    case TimerInfo::kNone: {
      // TODO (http://b/198135618): Create tracks only before drawing.
      track_manager->GetOrCreateThreadTrack(timer_info.thread_id());
      thread_track_data_provider_->AddTimer(timer_info);
      break;
    }
    case TimerInfo::kApiScope: {
      // TODO (http://b/198135618): Create tracks only before drawing.
      track_manager->GetOrCreateThreadTrack(timer_info.thread_id());
      thread_track_data_provider_->AddTimer(timer_info);
      break;
    }
    case TimerInfo::kApiScopeAsync: {
      ProcessAsyncTimer(timer_info);
      break;
    }
    default:
      ORBIT_UNREACHABLE();
  }

  RequestUpdate();
}

void TimeGraph::ProcessApiStringEvent(const orbit_client_protos::ApiStringEvent& string_event) {
  manual_instrumentation_manager_->ProcessStringEvent(string_event);
}

void TimeGraph::ProcessApiTrackValueEvent(const orbit_client_protos::ApiTrackValue& track_event) {
  VariableTrack* track = GetTrackManager()->GetOrCreateVariableTrack(track_event.name());

  uint64_t time = track_event.timestamp_ns();

  switch (track_event.data_case()) {
    case ApiTrackValue::kDataDouble:
      track->AddValue(time, track_event.data_double());
      break;
    case ApiTrackValue::kDataFloat:
      track->AddValue(time, track_event.data_float());
      break;
    case ApiTrackValue::kDataInt:
      track->AddValue(time, track_event.data_int());
      break;
    case ApiTrackValue::kDataInt64:
      track->AddValue(time, track_event.data_int64());
      break;
    case ApiTrackValue::kDataUint:
      track->AddValue(time, track_event.data_uint());
      break;
    case ApiTrackValue::kDataUint64:
      track->AddValue(time, track_event.data_uint64());
      break;
    default:
      ORBIT_UNREACHABLE();
  }
}

void TimeGraph::ProcessSystemMemoryTrackingTimer(const TimerInfo& timer_info) {
  SystemMemoryTrack* track = GetTrackManager()->GetSystemMemoryTrack();
  if (track == nullptr) {
    track = GetTrackManager()->CreateAndGetSystemMemoryTrack();
  }
  track->OnTimer(timer_info);

  if (absl::GetFlag(FLAGS_enable_warning_threshold) && !track->GetWarningThreshold().has_value()) {
    constexpr double kMegabytesToKilobytes = 1024.0;
    // TODO(b/216245594): This uses the DataManager not from the main thread, failing a check.
    double warning_threshold_mb =
        static_cast<double>(app_->GetMemoryWarningThresholdKb()) / kMegabytesToKilobytes;
    track->SetWarningThreshold(warning_threshold_mb);
  }
}

void TimeGraph::ProcessCGroupAndProcessMemoryTrackingTimer(const TimerInfo& timer_info) {
  uint64_t cgroup_name_hash = timer_info.registers(static_cast<size_t>(
      CaptureEventProcessor::CGroupAndProcessMemoryUsageEncodingIndex::kCGroupNameHash));
  std::string cgroup_name = app_->GetStringManager()->Get(cgroup_name_hash).value_or("");
  if (cgroup_name.empty()) return;

  CGroupAndProcessMemoryTrack* track = GetTrackManager()->GetCGroupAndProcessMemoryTrack();
  if (track == nullptr) {
    track = GetTrackManager()->CreateAndGetCGroupAndProcessMemoryTrack(cgroup_name);
  }
  track->OnTimer(timer_info);
}

void TimeGraph::ProcessPageFaultsTrackingTimer(const TimerInfo& timer_info) {
  uint64_t cgroup_name_hash = timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::PageFaultsEncodingIndex::kCGroupNameHash));
  std::string cgroup_name = app_->GetStringManager()->Get(cgroup_name_hash).value_or("");
  if (cgroup_name.empty()) return;

  PageFaultsTrack* track = GetTrackManager()->GetPageFaultsTrack();
  if (track == nullptr) {
    uint64_t memory_sampling_period_ms = app_->GetMemorySamplingPeriodMs();
    track = GetTrackManager()->CreateAndGetPageFaultsTrack(cgroup_name, memory_sampling_period_ms);
  }
  ORBIT_CHECK(track != nullptr);
  track->OnTimer(timer_info);
}

void TimeGraph::ProcessAsyncTimer(const TimerInfo& timer_info) {
  const std::string& track_name = timer_info.api_scope_name();
  AsyncTrack* track = GetTrackManager()->GetOrCreateAsyncTrack(track_name);
  track->OnTimer(timer_info);
}

float TimeGraph::GetWorldFromTick(uint64_t time) const {
  if (time_window_us_ > 0) {
    double start = TicksToMicroseconds(capture_min_timestamp_, time) - min_time_us_;
    double normalized_start = start / time_window_us_;
    float pos = static_cast<float>(normalized_start * GetWidth());
    return pos;
  }

  return 0;
}

float TimeGraph::GetWorldFromUs(double micros) const {
  return GetWorldFromTick(GetTickFromUs(micros));
}

double TimeGraph::GetUsFromTick(uint64_t time) const {
  return TicksToMicroseconds(capture_min_timestamp_, time) - min_time_us_;
}

uint64_t TimeGraph::GetNsSinceStart(uint64_t time) const { return time - capture_min_timestamp_; }

uint64_t TimeGraph::GetTickFromWorld(float world_x) const {
  double ratio = GetWidth() > 0 ? static_cast<double>(world_x / GetWidth()) : 0;
  auto time_span_ns = static_cast<uint64_t>(1000 * GetTime(ratio));
  return capture_min_timestamp_ + time_span_ns;
}

uint64_t TimeGraph::GetTickFromUs(double micros) const {
  auto nanos = static_cast<uint64_t>(1000 * micros);
  return capture_min_timestamp_ + nanos;
}

// Select a timer_info. Also move the view in order to assure that the timer_info and its track are
// visible.
void TimeGraph::SelectAndMakeVisible(const TimerInfo* timer_info) {
  ORBIT_CHECK(timer_info != nullptr);
  app_->SelectTimer(timer_info);
  HorizontallyMoveIntoView(VisibilityType::kPartlyVisible, *timer_info);
  track_container_->VerticallyMoveIntoView(*timer_info);
}

const TimerInfo* TimeGraph::FindPreviousFunctionCall(uint64_t function_address,
                                                     uint64_t current_time,
                                                     std::optional<uint32_t> thread_id) const {
  const TimerInfo* previous_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::lowest();
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      if (!block.Intersects(goal_time, current_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        auto timer_end_time = timer_info.end();
        if ((timer_info.function_id() == function_address) &&
            (!thread_id || thread_id.value() == timer_info.thread_id()) &&
            (timer_end_time < current_time) && (goal_time < timer_end_time)) {
          previous_timer = &timer_info;
          goal_time = timer_end_time;
        }
      }
    }
  }
  return previous_timer;
}

const TimerInfo* TimeGraph::FindNextFunctionCall(uint64_t function_address, uint64_t current_time,
                                                 std::optional<uint32_t> thread_id) const {
  const TimerInfo* next_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::max();
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      if (!block.Intersects(current_time, goal_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        auto timer_end_time = timer_info.end();
        if ((timer_info.function_id() == function_address) &&
            (!thread_id || thread_id.value() == timer_info.thread_id()) &&
            (timer_end_time > current_time) && (goal_time > timer_end_time)) {
          next_timer = &timer_info;
          goal_time = timer_end_time;
        }
      }
    }
  }
  return next_timer;
}

std::vector<const TimerInfo*> TimeGraph::GetAllTimersForHookedFunction(
    uint64_t function_address) const {
  std::vector<const TimerInfo*> timers;
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer = block[i];
        if (timer.function_id() == function_address) timers.push_back(&timer);
      }
    }
  }
  return timers;
}

std::vector<const TimerChain*> TimeGraph::GetAllThreadTrackTimerChains() const {
  return thread_track_data_provider_->GetAllThreadTimerChains();
}

std::pair<const TimerInfo*, const TimerInfo*> TimeGraph::GetMinMaxTimerInfoForFunction(
    uint64_t function_id) const {
  const TimerInfo* min_timer = nullptr;
  const TimerInfo* max_timer = nullptr;
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      for (size_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        if (timer_info.function_id() != function_id) continue;

        uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
        if (min_timer == nullptr || elapsed_nanos < (min_timer->end() - min_timer->start())) {
          min_timer = &timer_info;
        }
        if (max_timer == nullptr || elapsed_nanos > (max_timer->end() - max_timer->start())) {
          max_timer = &timer_info;
        }
      }
    }
  }
  return std::make_pair(min_timer, max_timer);
}

void TimeGraph::RequestUpdate() {
  CaptureViewElement::RequestUpdate();
  update_primitives_requested_ = true;
}

void TimeGraph::PrepareBatcherAndUpdatePrimitives(PickingMode picking_mode) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(app_->GetStringManager() != nullptr);

  batcher_.StartNewFrame();

  text_renderer_static_.Clear();

  uint64_t min_tick = GetTickFromUs(min_time_us_);
  uint64_t max_tick = GetTickFromUs(max_time_us_);

  CaptureViewElement::UpdatePrimitives(batcher_, text_renderer_static_, min_tick, max_tick,
                                       picking_mode);

  if (!absl::GetFlag(FLAGS_enforce_full_redraw)) {
    update_primitives_requested_ = false;
  }
}

void TimeGraph::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  capture_min_timestamp_ =
      std::min(capture_min_timestamp_, capture_data_->GetCallstackData().min_time());
  capture_max_timestamp_ =
      std::max(capture_max_timestamp_, capture_data_->GetCallstackData().max_time());

  time_window_us_ = max_time_us_ - min_time_us_;

  // Update position and size of children.
  track_container_->SetPos(GetPos()[0], GetPos()[1]);
  // TrackContainer height is the free space of the screen.
  track_container_->SetHeight(GetHeight() - timeline_ui_->GetHeight());
  timeline_ui_->SetPos(GetPos()[0], GetPos()[1] + track_container_->GetHeight());
}

void TimeGraph::SelectAndZoom(const TimerInfo* timer_info) {
  ORBIT_CHECK(timer_info);
  Zoom(*timer_info);
  SelectAndMakeVisible(timer_info);
}

void TimeGraph::JumpToNeighborTimer(const TimerInfo* from, JumpDirection jump_direction,
                                    JumpScope jump_scope) {
  if (from == nullptr || !TrackManager::IteratableType(from->type())) {
    return;
  }
  if ((jump_scope == JumpScope::kSameFunction ||
       jump_scope == JumpScope::kSameThreadSameFunction) &&
      !TrackManager::FunctionIteratableType(from->type())) {
    jump_scope = JumpScope::kSameDepth;
  }
  const TimerInfo* goal = nullptr;
  auto function_id = from->function_id();
  auto current_time = from->end();
  auto thread_id = from->thread_id();
  if (jump_direction == JumpDirection::kPrevious) {
    switch (jump_scope) {
      case JumpScope::kSameDepth:
        goal = track_container_->FindPrevious(*from);
        break;
      case JumpScope::kSameFunction:
        goal = FindPreviousFunctionCall(function_id, current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindPreviousFunctionCall(function_id, current_time, thread_id);
        break;
      default:
        // Other choices are not implemented.
        ORBIT_UNREACHABLE();
    }
  }
  if (jump_direction == JumpDirection::kNext) {
    switch (jump_scope) {
      case JumpScope::kSameDepth:
        goal = track_container_->FindNext(*from);
        break;
      case JumpScope::kSameFunction:
        goal = FindNextFunctionCall(function_id, current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindNextFunctionCall(function_id, current_time, thread_id);
        break;
      default:
        ORBIT_UNREACHABLE();
    }
  }
  if (jump_direction == JumpDirection::kTop) {
    goal = track_container_->FindTop(*from);
  }
  if (jump_direction == JumpDirection::kDown) {
    goal = track_container_->FindDown(*from);
  }
  if (goal != nullptr) {
    SelectAndMakeVisible(goal);
  }
}

void TimeGraph::DrawAllElements(Batcher& batcher, TextRenderer& text_renderer,
                                PickingMode& picking_mode, uint64_t current_mouse_time_ns) {
  const bool picking = picking_mode != PickingMode::kNone;

  DrawContext context{current_mouse_time_ns, picking_mode};
  Draw(batcher, text_renderer, context);

  if ((!picking && update_primitives_requested_) || picking) {
    PrepareBatcherAndUpdatePrimitives(picking_mode);
  }
}

void TimeGraph::DrawText(float layer) { text_renderer_static_.RenderLayer(layer); }

bool TimeGraph::IsFullyVisible(uint64_t min, uint64_t max) const {
  double start = TicksToMicroseconds(capture_min_timestamp_, min);
  double end = TicksToMicroseconds(capture_min_timestamp_, max);

  return start > min_time_us_ && end < max_time_us_;
}

bool TimeGraph::IsPartlyVisible(uint64_t min, uint64_t max) const {
  double start = TicksToMicroseconds(capture_min_timestamp_, min);
  double end = TicksToMicroseconds(capture_min_timestamp_, max);

  return !(min_time_us_ > end || max_time_us_ < start);
}

bool TimeGraph::IsVisible(VisibilityType vis_type, uint64_t min, uint64_t max) const {
  switch (vis_type) {
    case VisibilityType::kPartlyVisible:
      return IsPartlyVisible(min, max);
    case VisibilityType::kFullyVisible:
      return IsFullyVisible(min, max);
    default:
      return false;
  }
}

std::vector<orbit_gl::CaptureViewElement*> TimeGraph::GetAllChildren() const {
  return {GetTrackContainer(), GetTimelineUi()};
}

std::vector<orbit_gl::CaptureViewElement*> TimeGraph::GetNonHiddenChildren() const {
  return {GetTrackContainer(), GetTimelineUi()};
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TimeGraph::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::TimeGraphAccessibility>(this);
}
