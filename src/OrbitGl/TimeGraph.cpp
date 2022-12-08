// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TimeGraph.h"

#include <GteVector.h>
#include <absl/flags/flag.h>
#include <stddef.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "ApiInterface/Orbit.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "ClientData/CallstackData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeInfo.h"
#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"
#include "OrbitGl/AccessibleTimeGraph.h"
#include "OrbitGl/AsyncTrack.h"
#include "OrbitGl/CGroupAndProcessMemoryTrack.h"
#include "OrbitGl/FrameTrack.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/GpuTrack.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PageFaultsTrack.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/SchedulerTrack.h"
#include "OrbitGl/SystemMemoryTrack.h"
#include "OrbitGl/TrackManager.h"
#include "OrbitGl/VariableTrack.h"
#include "StringManager/StringManager.h"

using orbit_capture_client::CaptureEventProcessor;

using orbit_client_data::CaptureData;
using orbit_client_data::TimerChain;
using orbit_client_protos::TimerInfo;

using orbit_gl::Button;
using orbit_gl::CGroupAndProcessMemoryTrack;
using orbit_gl::PageFaultsTrack;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::SystemMemoryTrack;
using orbit_gl::TextRenderer;
using orbit_gl::TrackManager;
using orbit_gl::VariableTrack;

TimeGraph::TimeGraph(AccessibleInterfaceProvider* parent, OrbitApp* app,
                     orbit_gl::Viewport* viewport, CaptureData* capture_data,
                     PickingManager* picking_manager, TimeGraphLayout* time_graph_layout)
    // Note that `GlCanvas` and `TimeGraph` span the bridge to OpenGl content, and `TimeGraph`'s
    // parent needs special handling for accessibility. Thus, we use `nullptr` here and we save the
    // parent in accessible_parent_ which doesn't need to be a CaptureViewElement.
    : orbit_gl::CaptureViewElement(nullptr, viewport, time_graph_layout),
      accessible_parent_{parent},
      layout_{time_graph_layout},
      batcher_(BatcherId::kTimeGraph),
      primitive_assembler_(&batcher_, picking_manager),
      thread_track_data_provider_(capture_data->GetThreadTrackDataProvider()),
      capture_data_{capture_data},
      app_{app} {
  if (app_ != nullptr) {
    manual_instrumentation_manager_ = app->GetManualInstrumentationManager();
  }
  text_renderer_static_.SetViewport(viewport);

  const orbit_client_data::ModuleManager* module_manager =
      app != nullptr ? app->GetModuleManager() : nullptr;

  track_container_ = std::make_unique<orbit_gl::TrackContainer>(this, this, viewport, layout_, app_,
                                                                module_manager, capture_data);
  timeline_ui_ = std::make_unique<orbit_gl::TimelineUi>(
      /*parent=*/this, /*timeline_info_interface=*/this, viewport, layout_);

  horizontal_slider_ = std::make_shared<orbit_gl::GlHorizontalSlider>(
      /*parent=*/this, viewport, layout_, /*timeline_info_interface=*/this);
  vertical_slider_ = std::make_shared<orbit_gl::GlVerticalSlider>(
      /*parent=*/this, viewport, layout_, /*timeline_info_interface=*/this);

  horizontal_slider_->SetDragCallback([&](float ratio) { this->UpdateHorizontalScroll(ratio); });
  horizontal_slider_->SetResizeCallback([&](float normalized_start, float normalized_end) {
    this->UpdateHorizontalZoom(normalized_start, normalized_end);
  });

  vertical_slider_->SetDragCallback(
      [&](float ratio) { track_container_->UpdateVerticalScrollUsingRatio(ratio); });

  plus_button_ = std::make_shared<Button>(/*parent=*/this, viewport, layout_, "Plus Button",
                                          Button::SymbolType::kPlusSymbol);
  plus_button_->SetMouseReleaseCallback(
      [&](Button* /*button*/) { ZoomTime(/*zoom_value=*/1, /*mouse_ratio=*/0.5); });

  minus_button_ = std::make_shared<Button>(/*parent=*/this, viewport, layout_, "Minus Button",
                                           Button::SymbolType::kMinusSymbol);
  minus_button_->SetMouseReleaseCallback(
      [&](Button* /*button*/) { ZoomTime(/*zoom_value=*/-1, /*mouse_ratio=*/0.5); });

  if (absl::GetFlag(FLAGS_enforce_full_redraw)) {
    RequestUpdate();
  }
}

float TimeGraph::GetHeight() const { return viewport_->GetWorldHeight(); }

void TimeGraph::UpdateCaptureMinMaxTimestamps() {
  auto [tracks_min_time, tracks_max_time] = GetTrackManager()->GetTracksMinMaxTimestamps();

  uint64_t capture_min_timestamp = std::min(capture_min_timestamp_, tracks_min_time);
  uint64_t capture_max_timestamp = std::max(capture_max_timestamp_, tracks_max_time);

  capture_min_timestamp =
      std::min(capture_min_timestamp, capture_data_->GetCallstackData().min_time());
  capture_max_timestamp =
      std::max(capture_max_timestamp, capture_data_->GetCallstackData().max_time());

  if (capture_min_timestamp == capture_min_timestamp_ &&
      capture_max_timestamp == capture_max_timestamp_)
    return;

  capture_min_timestamp_ = capture_min_timestamp;
  capture_max_timestamp_ = capture_max_timestamp;

  RequestUpdate();
}

void TimeGraph::ZoomAll() {
  constexpr double kNumHistorySeconds = 2.f;
  UpdateCaptureMinMaxTimestamps();

  double max_time_us = TicksToMicroseconds(capture_min_timestamp_, capture_max_timestamp_);
  double min_time_us = max_time_us - (kNumHistorySeconds * 1000 * 1000);

  SetMinMax(min_time_us, max_time_us);
}

void TimeGraph::Zoom(uint64_t min, uint64_t max) {
  double start = TicksToMicroseconds(capture_min_timestamp_, min);
  double end = TicksToMicroseconds(capture_min_timestamp_, max);

  double mid = start + ((end - start) / 2.0);
  double extent = 1.1 * (end - start) / 2.0;

  SetMinMax(mid - extent, mid + extent);
}

void TimeGraph::Zoom(const TimerInfo& timer_info) { Zoom(timer_info.start(), timer_info.end()); }

double TimeGraph::GetCaptureTimeSpanUs() const { return GetCaptureTimeSpanNs() * 0.001; }

void TimeGraph::ZoomTime(int zoom_delta, double center_time_ratio) {
  constexpr double kIncrementalZoomTimeRatio = 0.1;
  double scale =
      (zoom_delta > 0) ? (1 + kIncrementalZoomTimeRatio) : (1 / (1 + kIncrementalZoomTimeRatio));
  // The horizontal zoom could have been triggered from the margin of TimeGraph, so we clamp the
  // mouse_ratio to ensure it is between 0 and 1.
  center_time_ratio = std::clamp(center_time_ratio, 0., 1.);

  double current_time_window_us = max_time_us_ - min_time_us_;
  ref_time_us_ = min_time_us_ + center_time_ratio * current_time_window_us;

  double time_left = std::max(ref_time_us_ - min_time_us_, 0.0);
  double time_right = std::max(max_time_us_ - ref_time_us_, 0.0);

  double min_time_us = ref_time_us_ - time_left / scale;
  double max_time_us = ref_time_us_ + time_right / scale;

  const double duration = max_time_us - min_time_us;

  if (duration < kTimeGraphMinTimeWindowsUs) {
    const double diff = kTimeGraphMinTimeWindowsUs - duration;
    min_time_us -= diff * center_time_ratio;
    max_time_us += diff * (1. - center_time_ratio);
  }

  SetMinMax(min_time_us, max_time_us);
}

void TimeGraph::VerticalZoom(float zoom_value, float mouse_world_y_pos) {
  constexpr float kIncrementRatio = 0.1f;
  const float proposed_ratio =
      (zoom_value > 0) ? (1 + kIncrementRatio) : (1 / (1 + kIncrementRatio));

  // We have to scale every item in the layout.
  const float old_scale = layout_->GetScale();
  layout_->SetScale(old_scale * proposed_ratio);

  // As we have maximum/minimum scale, the real ratio might be different than the proposed one.
  const float real_ratio = layout_->GetScale() / old_scale;

  // TODO(b/214270440): Behave differently when the mouse is on top of the timeline.
  track_container_->VerticalZoom(real_ratio, mouse_world_y_pos);
}

// TODO(b/214280802): include SetMinMax in the TimelineInfoInterface, so the scrollbar could call
// it.
void TimeGraph::SetMinMax(double min_time_us, double max_time_us) {
  // Center the interval on screen if needed
  if (max_time_us - min_time_us < kTimeGraphMinTimeWindowsUs || min_time_us < 0. ||
      max_time_us > GetCaptureTimeSpanUs()) {
    const double desired_time_window =
        std::max(max_time_us - min_time_us, kTimeGraphMinTimeWindowsUs);
    const double center_time_us = (max_time_us + min_time_us) / 2.;

    min_time_us = std::max(center_time_us - desired_time_window / 2, 0.0);
    max_time_us = std::min(min_time_us + desired_time_window, GetCaptureTimeSpanUs());
  }

  if (min_time_us == min_time_us_ && max_time_us == max_time_us_) return;

  min_time_us_ = min_time_us;
  max_time_us_ = max_time_us;

  RequestUpdate();
}

void TimeGraph::PanTime(int initial_x, int current_x, int width, double initial_time) {
  double time_window_us = max_time_us_ - min_time_us_;
  double initial_local_time = static_cast<double>(initial_x) / width * time_window_us;
  double dt = static_cast<double>(current_x - initial_x) / width * time_window_us;
  double current_time = initial_time - dt;
  double min_time_us =
      std::clamp(current_time - initial_local_time, 0.0, GetCaptureTimeSpanUs() - time_window_us);
  double max_time_us = min_time_us + time_window_us;

  SetMinMax(min_time_us, max_time_us);
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
  double min_time_us = ratio * (time_span - time_window);
  double max_time_us = min_time_us + time_window;

  SetMinMax(min_time_us, max_time_us);
}

void TimeGraph::UpdateHorizontalZoom(float normalized_start, float normalized_end) {
  SetMinMax(normalized_start * GetCaptureTimeSpanUs(), normalized_end * GetCaptureTimeSpanUs());
}

double TimeGraph::GetTime(double ratio) const {
  double current_width = max_time_us_ - min_time_us_;
  double delta = ratio * current_width;
  return min_time_us_ + delta;
}

void TimeGraph::ProcessTimer(const TimerInfo& timer_info) {
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
      if (timer_info.function_id() == orbit_grpc_protos::kInvalidFunctionId) {
        break;
      }
      FrameTrack* track = track_manager->GetOrCreateFrameTrack(timer_info.function_id());
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

void TimeGraph::ProcessApiStringEvent(const orbit_client_data::ApiStringEvent& string_event) {
  manual_instrumentation_manager_->ProcessStringEvent(string_event);
}

void TimeGraph::ProcessApiTrackValueEvent(
    const orbit_client_data::ApiTrackValue& track_event) const {
  VariableTrack* track = GetTrackManager()->GetOrCreateVariableTrack(track_event.track_name());

  uint64_t time = track_event.timestamp_ns();
  track->AddValue(time, track_event.value());
}

void TimeGraph::ProcessSystemMemoryInfo(
    const orbit_client_data::SystemMemoryInfo& system_memory_info) {
  SystemMemoryTrack* track = GetTrackManager()->GetSystemMemoryTrack();
  if (track == nullptr) {
    track = GetTrackManager()->CreateAndGetSystemMemoryTrack();
  }
  track->OnSystemMemoryInfo(system_memory_info);

  if (absl::GetFlag(FLAGS_enable_warning_threshold) && !track->GetWarningThreshold().has_value()) {
    constexpr double kMegabytesToKilobytes = 1024.0;
    double warning_threshold_mb =
        static_cast<double>(app_->GetMemoryWarningThresholdKb()) / kMegabytesToKilobytes;
    track->SetWarningThreshold(warning_threshold_mb);
  }
}

void TimeGraph::ProcessCgroupAndProcessMemoryInfo(
    const orbit_client_data::CgroupAndProcessMemoryInfo& cgroup_and_process_memory_info) {
  std::string cgroup_name =
      app_->GetStringManager()->Get(cgroup_and_process_memory_info.cgroup_name_hash).value_or("");
  if (cgroup_name.empty()) return;

  CGroupAndProcessMemoryTrack* track = GetTrackManager()->GetCGroupAndProcessMemoryTrack();
  if (track == nullptr) {
    track = GetTrackManager()->CreateAndGetCGroupAndProcessMemoryTrack(cgroup_name);
  }
  track->OnCgroupAndProcessMemoryInfo(cgroup_and_process_memory_info);
}

void TimeGraph::ProcessPageFaultsInfo(const orbit_client_data::PageFaultsInfo& page_faults_info) {
  std::string cgroup_name =
      app_->GetStringManager()->Get(page_faults_info.cgroup_name_hash).value_or("");
  if (cgroup_name.empty()) return;

  PageFaultsTrack* track = GetTrackManager()->GetPageFaultsTrack();
  if (track == nullptr) {
    uint64_t memory_sampling_period_ms = app_->GetMemorySamplingPeriodMs();
    track = GetTrackManager()->CreateAndGetPageFaultsTrack(cgroup_name, memory_sampling_period_ms);
  }
  ORBIT_CHECK(track != nullptr);
  track->OnPageFaultsInfo(page_faults_info);
}

orbit_gl::CaptureViewElement::EventResult TimeGraph::OnMouseWheel(
    const Vec2& mouse_pos, int delta, const orbit_gl::ModifierKeys& modifiers) {
  if (delta == 0) return EventResult::kIgnored;
  constexpr float kScrollingRatioPerDelta = 0.05f;

  if (modifiers.ctrl) {
    double mouse_ratio = (mouse_pos[0] - GetTimelinePos()[0]) / GetTimelineWidth();
    ZoomTime(delta, mouse_ratio);
  } else {
    track_container_->IncrementVerticalScroll(delta * kScrollingRatioPerDelta);
  }

  return EventResult::kHandled;
}

orbit_gl::CaptureViewElement::EventResult TimeGraph::OnMouseLeave() {
  EventResult event_result = CaptureViewElement::OnMouseLeave();
  // A redraw is needed since the mouse green line won't be drawn anymore.
  RequestUpdate(RequestUpdateScope::kDraw);
  return event_result;
}

void TimeGraph::ProcessAsyncTimer(const TimerInfo& timer_info) const {
  const std::string& track_name = timer_info.api_scope_name();
  AsyncTrack* track = GetTrackManager()->GetOrCreateAsyncTrack(track_name);
  track->OnTimer(timer_info);
}

float TimeGraph::GetWorldFromTick(uint64_t time) const {
  double time_window_us = GetTimeWindowUs();
  if (time_window_us > 0) {
    double start = TicksToMicroseconds(capture_min_timestamp_, time) - min_time_us_;
    double normalized_start = start / time_window_us;
    float pos =
        layout_->GetLeftMargin() + static_cast<float>(normalized_start * GetTimelineWidth());
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
  float relative_x = world_x - GetTimelinePos()[0];
  double ratio = GetTimelineWidth() > 0 ? static_cast<double>(relative_x / GetTimelineWidth()) : 0;
  auto time_span_ns = static_cast<uint64_t>(1000 * GetTime(ratio));
  return capture_min_timestamp_ + time_span_ns;
}

uint64_t TimeGraph::GetTickFromUs(double micros) const {
  auto nanos = static_cast<uint64_t>(1000 * micros);
  return capture_min_timestamp_ + nanos;
}

uint64_t TimeGraph::GetCaptureTimeSpanNs() const {
  // Do we have an empty capture?
  if (capture_max_timestamp_ == 0 &&
      capture_min_timestamp_ == std::numeric_limits<uint64_t>::max()) {
    return 0;
  }
  ORBIT_CHECK(capture_min_timestamp_ <= capture_max_timestamp_);
  return capture_max_timestamp_ - capture_min_timestamp_;
}

std::pair<float, float> TimeGraph::GetBoxPosXAndWidthFromTicks(uint64_t start_tick,
                                                               uint64_t end_tick) const {
  // TODO(b/244736453): GetWorldFromTick uses floats and therefore is not precise enough. Since
  //  the optimization looks for the first timer after the boundary of a pixel, we are getting
  //  several values very close to that boundary. The lack of precision is making some of that
  //  numbers to be just before the boundary and they ended be floored in the previous pixel. We
  //  are temporarily hacking this issue by adding an epsilon.
  // Epsilon for any float in the range of (0, 8092), maximum width for a 8k pixel screen.
  constexpr float kEpsilon = std::numeric_limits<float>::epsilon() * 8092;
  const float extended_start_x = std::floor(GetWorldFromTick(start_tick) + kEpsilon);
  const float extended_end_x = std::ceil(GetWorldFromTick(end_tick) + kEpsilon);
  return {extended_start_x, extended_end_x - extended_start_x};
}

// Select a timer_info. Also move the view in order to assure that the timer_info and its track are
// visible.
void TimeGraph::SelectAndMakeVisible(const TimerInfo* timer_info) {
  ORBIT_CHECK(timer_info != nullptr);
  app_->SelectTimer(timer_info);
  HorizontallyMoveIntoView(VisibilityType::kPartlyVisible, *timer_info);
  track_container_->VerticallyMoveIntoView(*timer_info);
}

static bool ThreadMatches(const std::optional<uint32_t>& target_thread_id, const TimerInfo* timer) {
  return !target_thread_id || *target_thread_id == timer->thread_id();
}

static void UpdatePreviousTimerAndGoalTime(const TimerInfo** previous_timer, uint64_t* goal_time,
                                           const TimerInfo* current_timer, uint64_t current_time) {
  if ((current_timer->end() < current_time) && (*goal_time < current_timer->end())) {
    *previous_timer = current_timer;
    *goal_time = current_timer->end();
  }
}

static void UpdateNextTimerAndGoalTime(const TimerInfo** next_timer, uint64_t& goal_time,
                                       const TimerInfo* current_timer, uint64_t current_time) {
  if ((current_timer->end() > current_time) && (goal_time > current_timer->end())) {
    *next_timer = current_timer;
    goal_time = current_timer->end();
  }
}

const TimerInfo* TimeGraph::FindPreviousScopeTimer(ScopeId scope_id, uint64_t current_time,
                                                   std::optional<uint32_t> thread_id) const {
  const orbit_client_data::ScopeType type = capture_data_->GetScopeInfo(scope_id).GetType();
  if (type == orbit_client_data::ScopeType::kInvalid) return nullptr;

  // If the type of the timer in question is `kDynamicallyInstrumentedFunction` or `kApiScope`, it
  // is stored in Thread Track, which we have an efficient search for. That implementation relies on
  // blockchain structure and cannot be generalized for the other tracks.
  if (type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      type == orbit_client_data::ScopeType::kApiScope) {
    return FindPreviousThreadTrackTimer(scope_id, current_time, thread_id);
  }

  const TimerInfo* previous_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::lowest();

  std::vector<const TimerInfo*> timers = capture_data_->GetAllScopeTimers(
      {type}, std::numeric_limits<uint64_t>::lowest(), current_time);
  for (const TimerInfo* current_timer : timers) {
    if (ThreadMatches(thread_id, current_timer) &&
        capture_data_->ProvideScopeId(*current_timer) == scope_id) {
      UpdatePreviousTimerAndGoalTime(&previous_timer, &goal_time, current_timer, current_time);
    }
  }
  return previous_timer;
}

const TimerInfo* TimeGraph::FindNextScopeTimer(ScopeId scope_id, uint64_t current_time,
                                               std::optional<uint32_t> thread_id) const {
  const orbit_client_data::ScopeType type = capture_data_->GetScopeInfo(scope_id).GetType();
  if (type == orbit_client_data::ScopeType::kInvalid) return nullptr;

  if (type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      type == orbit_client_data::ScopeType::kApiScope) {
    return FindNextThreadTrackTimer(scope_id, current_time, thread_id);
  }

  const TimerInfo* next_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::max();

  std::vector<const TimerInfo*> timers = capture_data_->GetAllScopeTimers({type}, current_time);
  for (const TimerInfo* current_timer : timers) {
    if (ThreadMatches(thread_id, current_timer) &&
        capture_data_->ProvideScopeId(*current_timer) == scope_id) {
      UpdateNextTimerAndGoalTime(&next_timer, goal_time, current_timer, current_time);
    }
  }
  return next_timer;
}

const TimerInfo* TimeGraph::FindNextThreadTrackTimer(ScopeId scope_id, uint64_t current_time,
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
        if (ThreadMatches(thread_id, &timer_info) &&
            capture_data_->ProvideScopeId(timer_info) == scope_id) {
          UpdateNextTimerAndGoalTime(&next_timer, goal_time, &timer_info, current_time);
        }
      }
    }
  }
  return next_timer;
}

const TimerInfo* TimeGraph::FindPreviousThreadTrackTimer(ScopeId scope_id, uint64_t current_time,
                                                         std::optional<uint32_t> thread_id) const {
  const TimerInfo* previous_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::lowest();
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      if (!block.Intersects(goal_time, current_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        if (ThreadMatches(thread_id, &timer_info) &&
            capture_data_->ProvideScopeId(timer_info) == scope_id) {
          UpdatePreviousTimerAndGoalTime(&previous_timer, &goal_time, &timer_info, current_time);
        }
      }
    }
  }
  return previous_timer;
}

std::vector<const TimerChain*> TimeGraph::GetAllThreadTrackTimerChains() const {
  ORBIT_CHECK(thread_track_data_provider_ != nullptr);
  return thread_track_data_provider_->GetAllThreadTimerChains();
}

static void UpdateMinMaxTimers(const TimerInfo** min_timer, const TimerInfo** max_timer,
                               const TimerInfo* next_observed_timer) {
  uint64_t elapsed_nanos = next_observed_timer->end() - next_observed_timer->start();
  if (*min_timer == nullptr || elapsed_nanos < ((*min_timer)->end() - (*min_timer)->start())) {
    *min_timer = next_observed_timer;
  }
  if (*max_timer == nullptr || elapsed_nanos > ((*max_timer)->end() - (*max_timer)->start())) {
    *max_timer = next_observed_timer;
  }
}

std::pair<const TimerInfo*, const TimerInfo*> TimeGraph::GetMinMaxTimerForThreadTrackScope(
    ScopeId scope_id) const {
  const TimerInfo* min_timer = nullptr;
  const TimerInfo* max_timer = nullptr;
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      for (size_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        if (capture_data_->ProvideScopeId(timer_info) != scope_id) continue;
        UpdateMinMaxTimers(&min_timer, &max_timer, &timer_info);
      }
    }
  }
  return std::make_pair(min_timer, max_timer);
}

std::pair<const TimerInfo*, const TimerInfo*> TimeGraph::GetMinMaxTimerForScope(
    ScopeId scope_id) const {
  const orbit_client_data::ScopeType type = capture_data_->GetScopeInfo(scope_id).GetType();
  if (type == orbit_client_data::ScopeType::kInvalid) return {nullptr, nullptr};

  if (type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      type == orbit_client_data::ScopeType::kApiScope) {
    return GetMinMaxTimerForThreadTrackScope(scope_id);
  }

  const TimerInfo* min_timer = nullptr;
  const TimerInfo* max_timer = nullptr;
  for (const TimerInfo* timer_info : capture_data_->GetAllScopeTimers({type})) {
    if (capture_data_->ProvideScopeId(*timer_info) != scope_id) continue;
    UpdateMinMaxTimers(&min_timer, &max_timer, timer_info);
  }

  return std::make_pair(min_timer, max_timer);
}

void TimeGraph::PrepareBatcherAndUpdatePrimitives(PickingMode picking_mode) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(app_->GetStringManager() != nullptr);

  primitive_assembler_.StartNewFrame();

  text_renderer_static_.Init();
  text_renderer_static_.Clear();

  uint64_t min_tick = GetTickFromUs(min_time_us_);
  uint64_t max_tick = GetTickFromUs(max_time_us_);

  // Only update the primitives to draw if the time interval is non-empty.
  if (min_tick < max_tick) {
    CaptureViewElement::UpdatePrimitives(primitive_assembler_, text_renderer_static_, min_tick,
                                         max_tick, picking_mode);
  }

  if (absl::GetFlag(FLAGS_enforce_full_redraw)) {
    RequestUpdate();
  }
}

void TimeGraph::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();
  // Window resizing could have modified viewport's width.
  SetWidth(viewport_->GetWorldWidth());

  UpdateCaptureMinMaxTimestamps();
  UpdateChildrenPosAndContainerSize();
}

void TimeGraph::UpdateChildrenPosAndContainerSize() {
  // TimeGraph's children:
  // _______________________________________________
  // | L |            TIMELINE            | R |  +  |
  // | E |                                | I |  -  |
  // | F |--------------------------------| G |-----|
  // | T |     SPACE TRACKS - TIMELINE      H       |
  // |   |--------------------------------| T |-----|
  // | M |                                |   |  S  |
  // | A |                                | M |  L  |
  // | R |         TRACK CONTAINER        | A |  I  |
  // | G |                                | R |  D  |
  // | I |                                | G |  E  |
  // | N |                                | I |  R  |
  // |   |________________________________|_N_|_____|
  // |   |       HORIZONTAL SLIDER            |
  // |----------------------------------------|

  // First we calculate TrackContainer's height. TimeGraph will set TrackContainer height based on
  // its free space.
  float total_height_without_track_container = GetHeight() - horizontal_slider_->GetHeight() -
                                               timeline_ui_->GetHeight() -
                                               layout_->GetSpaceBetweenTracksAndTimeline();
  track_container_->SetHeight(total_height_without_track_container);

  // After we set positions.
  float timegraph_current_x = GetPos()[0];
  float timegraph_current_y = GetPos()[1];
  const float total_right_margin = layout_->GetRightMargin() + vertical_slider_->GetWidth();

  timeline_ui_->SetWidth(GetWidth() - total_right_margin - layout_->GetLeftMargin());
  timeline_ui_->SetPos(timegraph_current_x + layout_->GetLeftMargin(), timegraph_current_y);

  plus_button_->SetWidth(layout_->GetButtonWidth());
  plus_button_->SetHeight(layout_->GetButtonHeight());
  plus_button_->SetPos(GetWidth() - plus_button_->GetWidth(), timegraph_current_y);

  minus_button_->SetWidth(layout_->GetButtonWidth());
  minus_button_->SetHeight(layout_->GetButtonHeight());
  minus_button_->SetPos(GetWidth() - minus_button_->GetWidth(),
                        timegraph_current_y + plus_button_->GetHeight());

  float x_offset = layout_->GetLeftMargin();
  timegraph_current_y += timeline_ui_->GetHeight() + layout_->GetSpaceBetweenTracksAndTimeline();
  track_container_->SetWidth(GetWidth() - total_right_margin - x_offset);
  track_container_->SetPos(timegraph_current_x + x_offset, timegraph_current_y);

  vertical_slider_->SetWidth(layout_->GetSliderWidth());
  vertical_slider_->SetPos(GetWidth() - vertical_slider_->GetWidth(), timegraph_current_y);

  timegraph_current_y += track_container_->GetHeight();
  float slider_width = GetWidth() - vertical_slider_->GetWidth() - layout_->GetLeftMargin();
  horizontal_slider_->SetWidth(slider_width);
  // The horizontal slider should be at the bottom of the TimeGraph. Because how OpenGl renders, the
  // way to assure that there is no pixels below the scrollbar is by making a ceiling.
  horizontal_slider_->SetPos(timegraph_current_x + layout_->GetLeftMargin(),
                             std::ceil(GetHeight() - horizontal_slider_->GetHeight()));

  // TODO(b/230442062): Refactor this to be part of Slider::UpdateLayout().
  UpdateHorizontalSliderFromWorld();
  UpdateVerticalSliderFromWorld();
}

float TimeGraph::GetRightMargin() const {
  return layout_->GetRightMargin() +
         (vertical_slider_->GetVisible() ? vertical_slider_->GetWidth() : 0.f);
}

void TimeGraph::UpdateHorizontalSliderFromWorld() {
  double time_span = GetCaptureTimeSpanUs();
  double start = GetMinTimeUs();
  double stop = GetMaxTimeUs();
  double width = stop - start;
  double max_start = time_span - width;

  constexpr double kEpsilon = 1e-8;
  double ratio = max_start > kEpsilon ? start / max_start : 0;
  horizontal_slider_->SetNormalizedLength(static_cast<float>(width / time_span));
  horizontal_slider_->SetNormalizedPosition(static_cast<float>(ratio));
}

void TimeGraph::UpdateVerticalSliderFromWorld() {
  float visible_tracks_height = GetTrackContainer()->GetVisibleTracksTotalHeight();
  float max_height = std::max(0.f, visible_tracks_height - vertical_slider_->GetHeight());
  float pos_ratio =
      max_height > 0 ? GetTrackContainer()->GetVerticalScrollingOffset() / max_height : 0.f;
  float size_ratio =
      visible_tracks_height > 0 ? vertical_slider_->GetHeight() / visible_tracks_height : 1.f;
  vertical_slider_->SetNormalizedPosition(pos_ratio);
  vertical_slider_->SetNormalizedLength(size_ratio);
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
  const std::optional<ScopeId> scope_id = capture_data_->ProvideScopeId(*from);
  if (!scope_id.has_value()) return;
  auto current_time = from->end();
  auto thread_id = from->thread_id();
  if (jump_direction == JumpDirection::kPrevious) {
    switch (jump_scope) {
      case JumpScope::kSameDepth:
        goal = track_container_->FindPrevious(*from);
        break;
      case JumpScope::kSameFunction:
        goal = FindPreviousScopeTimer(scope_id.value(), current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindPreviousScopeTimer(scope_id.value(), current_time, thread_id);
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
        goal = FindNextScopeTimer(scope_id.value(), current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindNextScopeTimer(scope_id.value(), current_time, thread_id);
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

void TimeGraph::DrawAllElements(PrimitiveAssembler& primitive_assembler,
                                TextRenderer& text_renderer, PickingMode& picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;

  std::optional<uint64_t> current_mouse_time_ns = GetTickFromWorld(mouse_pos_cur_[0]);
  // Check if the mouse is over the TimeGraph and inside the timeline visible time range.
  if (!IsMouseOver() ||
      !orbit_gl::ClosedInterval<uint64_t>{GetTickFromUs(min_time_us_), GetTickFromUs(max_time_us_)}
           .Contains(current_mouse_time_ns.value())) {
    current_mouse_time_ns = std::nullopt;
  }
  DrawContext context{current_mouse_time_ns, picking_mode};
  // `Draw` is called in any case - the batcher has already been cleared if this method is called,
  // so we need to re-fill it.
  Draw(primitive_assembler, text_renderer, context);

  if (picking || update_primitives_requested_) {
    PrepareBatcherAndUpdatePrimitives(picking_mode);
  }
}

void TimeGraph::DoDraw(orbit_gl::PrimitiveAssembler& primitive_assembler,
                       orbit_gl::TextRenderer& text_renderer, const DrawContext& draw_context) {
  ORBIT_SCOPE("TimeGraph::DoDraw");
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  // Vertical green line at mouse x position.
  if (draw_context.picking_mode == PickingMode::kNone &&
      draw_context.current_mouse_tick.has_value()) {
    const Color green_line_color{0, 255, 0, 127};
    Vec2 green_line_pos = {GetWorldFromTick(draw_context.current_mouse_tick.value()), GetPos()[1]};
    primitive_assembler.AddVerticalLine(green_line_pos, GetHeight(), GlCanvas::kZValueUi,
                                        green_line_color);
  }

  // TODO(http://b/217719000): We are drawing boxes in margin positions because some elements are
  // being drawn partially outside the TrackContainer space. This hack is needed until we assure
  // that no element is drawn outside of its parent's area.
  DrawMarginsBetweenChildren(primitive_assembler);
}

void TimeGraph::DrawMarginsBetweenChildren(
    orbit_gl::PrimitiveAssembler& primitive_assembler) const {
  // Margin between the Tracks and the Timeline.
  Vec2 timeline_margin_pos = Vec2(GetPos()[0], GetPos()[1] + timeline_ui_->GetHeight());
  Vec2 timeline_margin_size = Vec2(GetSize()[0], layout_->GetSpaceBetweenTracksAndTimeline());
  primitive_assembler.AddBox(MakeBox(timeline_margin_pos, timeline_margin_size),
                             GlCanvas::kZValueTimeBar, GlCanvas::kBackgroundColor);

  // Right margin mask for Timeline.
  float right_margin_width = GetRightMargin();
  float right_margin_height = GetHeight() - timeline_ui_->GetHeight();
  Vec2 right_margin_pos{GetWidth() - right_margin_width, GetPos()[1]};
  Quad right_margin_box = MakeBox(right_margin_pos, Vec2(right_margin_width, right_margin_height));
  primitive_assembler.AddBox(right_margin_box, GlCanvas::kZValueMargin, GlCanvas::kBackgroundColor);

  // Left margin mask for Timeline.
  float left_margin_width = layout_->GetLeftMargin();
  float left_margin_height = GetHeight();
  Vec2 left_margin_pos = GetPos();
  Quad left_margin_box = MakeBox(left_margin_pos, Vec2(left_margin_width, left_margin_height));
  primitive_assembler.AddBox(left_margin_box, GlCanvas::kZValueMargin, GlCanvas::kBackgroundColor);
}

void TimeGraph::DrawText(QPainter* painter, float layer) {
  text_renderer_static_.RenderLayer(painter, layer);
}

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
  return {GetTimelineUi(), GetTrackContainer(), GetHorizontalSlider(),
          GetPlusButton(), GetMinusButton(),    GetVerticalSlider()};
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TimeGraph::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTimeGraph>(this);
}
