// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraph.h"

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/time/time.h>
#include <stddef.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "App.h"
#include "AsyncTrack.h"
#include "CoreUtils.h"
#include "FrameTrack.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "GrpcProtos/Constants.h"
#include "ManualInstrumentationManager.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Tracing.h"
#include "OrbitClientData/FunctionUtils.h"
#include "PickingManager.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "TextBox.h"
#include "ThreadTrack.h"
#include "TrackManager.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::InstrumentedFunction;

using orbit_grpc_protos::kMissingInfo;

TimeGraph::TimeGraph(OrbitApp* app, TextRenderer* text_renderer, GlCanvas* canvas,
                     const CaptureData* capture_data)
    : text_renderer_{text_renderer},
      canvas_{canvas},
      accessibility_(this),
      batcher_(BatcherId::kTimeGraph),
      capture_data_{capture_data},
      app_{app} {
  text_renderer_->SetCanvas(canvas);
  text_renderer_static_.SetCanvas(canvas);
  batcher_.SetPickingManager(&canvas->GetPickingManager());
  track_manager_ = std::make_unique<TrackManager>(this, &GetLayout(), app, capture_data);

  async_timer_info_listener_ =
      std::make_unique<ManualInstrumentationManager::AsyncTimerInfoListener>(
          [this](const std::string& name, const TimerInfo& timer_info) {
            ProcessAsyncTimer(name, timer_info);
          });
  manual_instrumentation_manager_ = app_->GetManualInstrumentationManager();
  manual_instrumentation_manager_->AddAsyncTimerListener(async_timer_info_listener_.get());
}

TimeGraph::~TimeGraph() {
  manual_instrumentation_manager_->RemoveAsyncTimerListener(async_timer_info_listener_.get());
}

double GNumHistorySeconds = 2.f;

void TimeGraph::UpdateCaptureMinMaxTimestamps() {
  auto [tracks_min_time, tracks_max_time] = track_manager_->GetTracksMinMaxTimestamps();

  capture_min_timestamp_ = std::min(capture_min_timestamp_, tracks_min_time);
  capture_max_timestamp_ = std::max(capture_max_timestamp_, tracks_max_time);
}

void TimeGraph::ZoomAll() {
  UpdateCaptureMinMaxTimestamps();
  max_time_us_ = TicksToMicroseconds(capture_min_timestamp_, capture_max_timestamp_);
  min_time_us_ = max_time_us_ - (GNumHistorySeconds * 1000 * 1000);
  if (min_time_us_ < 0) min_time_us_ = 0;

  RequestUpdatePrimitives();
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
  CHECK(capture_min_timestamp_ <= capture_max_timestamp_);
  return TicksToMicroseconds(capture_min_timestamp_, capture_max_timestamp_);
}

double TimeGraph::GetCurrentTimeSpanUs() const { return max_time_us_ - min_time_us_; }

void TimeGraph::RequestRedraw() {
  redraw_requested_ = true;
  if (canvas_ != nullptr) {
    canvas_->RequestRedraw();
  }
}

void TimeGraph::ZoomTime(float zoom_value, double mouse_ratio) {
  static double increment_ratio = 0.1;
  double scale = (zoom_value > 0) ? (1 + increment_ratio) : (1 / (1 + increment_ratio));

  double current_time_window_us = max_time_us_ - min_time_us_;
  ref_time_us_ = min_time_us_ + mouse_ratio * current_time_window_us;

  double time_left = std::max(ref_time_us_ - min_time_us_, 0.0);
  double time_right = std::max(max_time_us_ - ref_time_us_, 0.0);

  double min_time_us = ref_time_us_ - scale * time_left;
  double max_time_us = ref_time_us_ + scale * time_right;

  if (max_time_us - min_time_us < 0.001 /*1 ns*/) {
    return;
  }

  SetMinMax(min_time_us, max_time_us);
}

void TimeGraph::VerticalZoom(float zoom_value, float mouse_relative_position) {
  constexpr float kIncrementRatio = 0.1f;

  const float ratio = (zoom_value > 0) ? (1 + kIncrementRatio) : (1 / (1 + kIncrementRatio));

  const float world_height = canvas_->GetWorldHeight();
  const float y_mouse_position =
      canvas_->GetWorldTopLeftY() - mouse_relative_position * world_height;
  const float top_distance = canvas_->GetWorldTopLeftY() - y_mouse_position;

  const float new_y_mouse_position = y_mouse_position / ratio;

  float new_world_top_left_y = new_y_mouse_position + top_distance;

  canvas_->UpdateWorldTopLeftY(new_world_top_left_y);

  // Finally, we have to scale every item in the layout.
  const float old_scale = layout_.GetScale();
  layout_.SetScale(old_scale / ratio);
}

void TimeGraph::SetMinMax(double min_time_us, double max_time_us) {
  double desired_time_window = max_time_us - min_time_us;
  min_time_us_ = std::max(min_time_us, 0.0);
  max_time_us_ = std::min(min_time_us_ + desired_time_window, GetCaptureTimeSpanUs());

  RequestUpdatePrimitives();
}

void TimeGraph::PanTime(int initial_x, int current_x, int width, double initial_time) {
  time_window_us_ = max_time_us_ - min_time_us_;
  double initial_local_time = static_cast<double>(initial_x) / width * time_window_us_;
  double dt = static_cast<double>(current_x - initial_x) / width * time_window_us_;
  double current_time = initial_time - dt;
  min_time_us_ =
      clamp(current_time - initial_local_time, 0.0, GetCaptureTimeSpanUs() - time_window_us_);
  max_time_us_ = min_time_us_ + time_window_us_;

  RequestUpdatePrimitives();
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

  RequestUpdatePrimitives();
}

void TimeGraph::HorizontallyMoveIntoView(VisibilityType vis_type, const TimerInfo& timer_info,
                                         double distance) {
  HorizontallyMoveIntoView(vis_type, timer_info.start(), timer_info.end(), distance);
}

void TimeGraph::VerticallyMoveIntoView(const TimerInfo& timer_info) {
  VerticallyMoveIntoView(*track_manager_->GetOrCreateThreadTrack(timer_info.thread_id()));
}

void TimeGraph::VerticallyMoveIntoView(Track& track) {
  float pos = track.GetPos()[1];
  float height = track.GetHeight();
  float world_top_left_y = canvas_->GetWorldTopLeftY();

  float min_world_top_left_y = pos + layout_.GetTrackTabHeight();
  float max_world_top_left_y = pos + canvas_->GetWorldHeight() - height - layout_.GetBottomMargin();
  canvas_->UpdateWorldTopLeftY(clamp(world_top_left_y, min_world_top_left_y, max_world_top_left_y));
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

  // Functions for manual instrumentation scopes and tracked values are those with orbit_type() !=
  // FunctionInfo::kNone. All proper timers for these have timer_info.type() == TimerInfo::kNone. It
  // is possible to add frame tracks for these special functions, as they are simply hooked
  // functions, but in those cases the timer type is TimerInfo::kFrame. We need to exclude those
  // frame track timers from the special processing here as they do not represent manual
  // instrumentation scopes.
  FunctionInfo::OrbitType orbit_type = FunctionInfo::kNone;
  if (function != nullptr) {
    orbit_type = function_utils::GetOrbitTypeByName(function->function_name());
  }

  if (function != nullptr && function_utils::IsOrbitFunctionFromType(orbit_type) &&
      timer_info.type() == TimerInfo::kNone) {
    ProcessOrbitFunctionTimer(orbit_type, timer_info);
  }

  // TODO (b/175869409): Change the way to create and get the tracks. Move this part to
  // TrackManager.
  switch (timer_info.type()) {
    // All GPU timers are handled equally here.
    case TimerInfo::kGpuActivity:
    case TimerInfo::kGpuCommandBuffer:
    case TimerInfo::kGpuDebugMarker: {
      uint64_t timeline_hash = timer_info.timeline_hash();
      GpuTrack* track = track_manager_->GetOrCreateGpuTrack(timeline_hash);
      track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kFrame: {
      if (function == nullptr) {
        break;
      }
      FrameTrack* track = track_manager_->GetOrCreateFrameTrack(*function);
      track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kIntrospection: {
      ProcessIntrospectionTimer(timer_info);
      break;
    }
    case TimerInfo::kCoreActivity: {
      // TODO(b/176962090): We need to create the `ThreadTrack` here even we don't use it, as we
      //  don't create it on new callstack events, yet.
      track_manager_->GetOrCreateThreadTrack(timer_info.thread_id());
      SchedulerTrack* scheduler_track = track_manager_->GetOrCreateSchedulerTrack();
      scheduler_track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kSystemMemoryUsage: {
      ProcessMemoryTrackingTimer(timer_info);
      break;
    }
    case TimerInfo::kNone: {
      ThreadTrack* track = track_manager_->GetOrCreateThreadTrack(timer_info.thread_id());
      track->OnTimer(timer_info);
      break;
    }
    case TimerInfo::kApiEvent: {
      ProcessApiEventTimer(timer_info);
      break;
    }
    default:
      UNREACHABLE();
  }

  RequestUpdatePrimitives();
}

void TimeGraph::ProcessOrbitFunctionTimer(FunctionInfo::OrbitType type,
                                          const TimerInfo& timer_info) {
  switch (type) {
    case FunctionInfo::kOrbitTrackValue:
      ProcessValueTrackingTimer(timer_info);
      break;
    case FunctionInfo::kOrbitTimerStartAsync:
    case FunctionInfo::kOrbitTimerStopAsync:
      manual_instrumentation_manager_->ProcessAsyncTimerDeprecated(timer_info);
      break;
    default:
      break;
  }
}

void TimeGraph::ProcessApiEventTimer(const TimerInfo& timer_info) {
  orbit_api::Event api_event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);
  switch (api_event.type) {
    case orbit_api::kScopeStart:
    case orbit_api::kScopeStop: {
      ThreadTrack* track = track_manager_->GetOrCreateThreadTrack(timer_info.thread_id());
      track->OnTimer(timer_info);
      break;
    }
    case orbit_api::kScopeStartAsync:
    case orbit_api::kScopeStopAsync:
      manual_instrumentation_manager_->ProcessAsyncTimer(timer_info);
      break;

    case orbit_api::kTrackInt:
    case orbit_api::kTrackInt64:
    case orbit_api::kTrackUint:
    case orbit_api::kTrackUint64:
    case orbit_api::kTrackFloat:
    case orbit_api::kTrackDouble:
    case orbit_api::kString:
      ProcessValueTrackingTimer(timer_info);
      break;
    case orbit_api::kNone:
      UNREACHABLE();
  }
}

void TimeGraph::ProcessIntrospectionTimer(const TimerInfo& timer_info) {
  orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);

  switch (event.type) {
    case orbit_api::kScopeStart: {
      ThreadTrack* track = track_manager_->GetOrCreateThreadTrack(timer_info.thread_id());
      track->OnTimer(timer_info);
    } break;
    case orbit_api::kScopeStartAsync:
    case orbit_api::kScopeStopAsync:
      manual_instrumentation_manager_->ProcessAsyncTimer(timer_info);
      break;
    case orbit_api::kTrackInt:
    case orbit_api::kTrackInt64:
    case orbit_api::kTrackUint:
    case orbit_api::kTrackUint64:
    case orbit_api::kTrackFloat:
    case orbit_api::kTrackDouble:
    case orbit_api::kString:
      ProcessValueTrackingTimer(timer_info);
      break;
    default:
      ERROR("Unhandled introspection type [%u]", event.type);
  }
}

void TimeGraph::ProcessValueTrackingTimer(const TimerInfo& timer_info) {
  orbit_api::Event event = ManualInstrumentationManager::ApiEventFromTimerInfo(timer_info);

  if (event.type == orbit_api::kString) {
    manual_instrumentation_manager_->ProcessStringEvent(event);
    return;
  }

  GraphTrack* track = track_manager_->GetOrCreateGraphTrack(event.name);
  uint64_t time = timer_info.start();

  switch (event.type) {
    case orbit_api::kTrackInt: {
      track->AddValue(orbit_api::Decode<int32_t>(event.data), time);
    } break;
    case orbit_api::kTrackInt64: {
      track->AddValue(orbit_api::Decode<int64_t>(event.data), time);
    } break;
    case orbit_api::kTrackUint: {
      track->AddValue(orbit_api::Decode<uint32_t>(event.data), time);
    } break;
    case orbit_api::kTrackUint64: {
      track->AddValue(event.data, time);
    } break;
    case orbit_api::kTrackFloat: {
      track->AddValue(orbit_api::Decode<float>(event.data), time);
    } break;
    case orbit_api::kTrackDouble: {
      track->AddValue(orbit_api::Decode<double>(event.data), time);
    } break;
    default:
      ERROR("Unsupported value tracking type [%u]", event.type);
      break;
  }

  if (track->GetProcessId() == -1) {
    track->SetProcessId(timer_info.process_id());
  }
}

void TimeGraph::ProcessMemoryTrackingTimer(const TimerInfo& timer_info) {
  const std::string kUnusedLabel = "System Memory Unused (MB)";
  const std::string kBuffersOrCachedLabel = "System Memory Buffers / Cached (MB)";
  const std::string kUsedLabel = "System Memory Used (MB)";
  const std::string kWarningThresholdLabel = "Production Limit";
  const std::string kValueUpperBoundLabel = "System Memory Total";
  const std::string kValueLowerBoundLabel = "Minimum: 0 GB";
  const std::string kTrackValueLabelUnit = "MB";

  constexpr double kValueLowerBoundRawValue = 0.0;
  constexpr uint8_t kTrackValueDecimalDigits = 2;
  constexpr uint64_t kKilobytesToBytes = 1024;
  constexpr double kMegabytesToKilobytes = 1024.0;

  CHECK(app_->GetCollectMemoryInfo());
  uint64_t warning_threshold_kb = app_->GetMemoryWarningThresholdKb();
  std::string warning_threshold_pretty_size =
      GetPrettySize(warning_threshold_kb * kKilobytesToBytes);
  std::string warning_threshold_pretty_label =
      absl::StrFormat("%s: %s", kWarningThresholdLabel, warning_threshold_pretty_size);
  double warning_threshold_raw_value =
      static_cast<double>(warning_threshold_kb) / kMegabytesToKilobytes;

  int64_t total_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(OrbitApp::SystemMemoryUsageEncodingIndex::kTotalKb)));
  std::string total_pretty_label;
  double total_raw_value = 0.0;
  if (total_kb != kMissingInfo) {
    std::string total_pretty_size = GetPrettySize(total_kb * kKilobytesToBytes);
    total_pretty_label = absl::StrFormat("%s: %s", kValueUpperBoundLabel, total_pretty_size);
    total_raw_value = static_cast<double>(total_kb) / kMegabytesToKilobytes;
  }

  GraphTrack* track;
  int64_t unused_kb = orbit_api::Decode<int64_t>(
      timer_info.registers(static_cast<size_t>(OrbitApp::SystemMemoryUsageEncodingIndex::kFreeKb)));
  int64_t buffers_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(OrbitApp::SystemMemoryUsageEncodingIndex::kBuffersKb)));
  int64_t cached_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(OrbitApp::SystemMemoryUsageEncodingIndex::kCachedKb)));
  if (buffers_kb != kMissingInfo && cached_kb != kMissingInfo) {
    track = track_manager_->GetOrCreateGraphTrack(kBuffersOrCachedLabel);
    if (total_kb != kMissingInfo) {
      track->SetValueUpperBoundWhenEmpty(total_pretty_label, total_raw_value);
    }
    track->SetValueLowerBoundWhenEmpty(kValueLowerBoundLabel, kValueLowerBoundRawValue);
    track->SetLabelUnitWhenEmpty(kTrackValueLabelUnit);
    track->SetValueDecimalDigitsWhenEmpty(kTrackValueDecimalDigits);
    double buffers_or_cached_mb =
        static_cast<double>(buffers_kb + cached_kb) / kMegabytesToKilobytes;
    track->AddValue(buffers_or_cached_mb, timer_info.start());
  }

  if (total_kb != kMissingInfo && unused_kb != kMissingInfo && buffers_kb != kMissingInfo &&
      cached_kb != kMissingInfo) {
    track = track_manager_->GetOrCreateGraphTrack(kUsedLabel);
    track->SetValueUpperBoundWhenEmpty(total_pretty_label, total_raw_value);
    track->SetValueLowerBoundWhenEmpty(kValueLowerBoundLabel, kValueLowerBoundRawValue);
    track->SetWarningThresholdWhenEmpty(warning_threshold_pretty_label,
                                        warning_threshold_raw_value);
    track->SetLabelUnitWhenEmpty(kTrackValueLabelUnit);
    track->SetValueDecimalDigitsWhenEmpty(kTrackValueDecimalDigits);
    double used_mb =
        static_cast<double>(total_kb - unused_kb - buffers_kb - cached_kb) / kMegabytesToKilobytes;
    track->AddValue(used_mb, timer_info.start());
  }
}

void TimeGraph::ProcessAsyncTimer(const std::string& track_name, const TimerInfo& timer_info) {
  AsyncTrack* track = track_manager_->GetOrCreateAsyncTrack(track_name);
  track->OnTimer(timer_info);
}

std::vector<std::shared_ptr<TimerChain>> TimeGraph::GetAllTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& track : track_manager_->GetAllTracks()) {
    Append(chains, track->GetAllChains());
  }
  // Frame tracks are removable by users and cannot simply be thrown into the
  // tracks_ vector.
  for (const auto& track : track_manager_->GetFrameTracks()) {
    Append(chains, track->GetAllChains());
  }
  return chains;
}

std::vector<std::shared_ptr<TimerChain>> TimeGraph::GetAllThreadTrackTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& track : track_manager_->GetThreadTracks()) {
    Append(chains, track->GetAllChains());
  }
  return chains;
}

std::vector<std::shared_ptr<TimerChain>> TimeGraph::GetAllSerializableTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& track : track_manager_->GetAllTracks()) {
    Append(chains, track->GetAllSerializableChains());
  }
  return chains;
}

float TimeGraph::GetWorldFromTick(uint64_t time) const {
  if (time_window_us_ > 0) {
    double start = TicksToMicroseconds(capture_min_timestamp_, time) - min_time_us_;
    double normalized_start = start / time_window_us_;
    auto pos = float(world_start_x_ + normalized_start * world_width_);
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

uint64_t TimeGraph::GetTickFromWorld(float world_x) const {
  double ratio =
      world_width_ != 0 ? static_cast<double>((world_x - world_start_x_) / world_width_) : 0;
  auto time_span_ns = static_cast<uint64_t>(1000 * GetTime(ratio));
  return capture_min_timestamp_ + time_span_ns;
}

uint64_t TimeGraph::GetTickFromUs(double micros) const {
  auto nanos = static_cast<uint64_t>(1000 * micros);
  return capture_min_timestamp_ + nanos;
}

void TimeGraph::GetWorldMinMax(float& min, float& max) const {
  min = GetWorldFromTick(capture_min_timestamp_);
  max = GetWorldFromTick(capture_max_timestamp_);
}

void TimeGraph::Select(const TextBox* text_box) {
  CHECK(text_box != nullptr);
  app_->SelectTextBox(text_box);
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  HorizontallyMoveIntoView(VisibilityType::kPartlyVisible, timer_info);
  VerticallyMoveIntoView(timer_info);
}

const TextBox* TimeGraph::FindPreviousFunctionCall(uint64_t function_id, uint64_t current_time,
                                                   std::optional<int32_t> thread_id) const {
  const TextBox* previous_box = nullptr;
  uint64_t previous_box_time = std::numeric_limits<uint64_t>::lowest();
  std::vector<std::shared_ptr<TimerChain>> chains = GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (const auto& block : *chain) {
      if (!block.Intersects(previous_box_time, current_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TextBox& box = block[i];
        auto box_time = box.GetTimerInfo().end();
        if ((box.GetTimerInfo().function_id() == function_id) &&
            (!thread_id || thread_id.value() == box.GetTimerInfo().thread_id()) &&
            (box_time < current_time) && (previous_box_time < box_time)) {
          previous_box = &box;
          previous_box_time = box_time;
        }
      }
    }
  }
  return previous_box;
}

const TextBox* TimeGraph::FindNextFunctionCall(uint64_t function_id, uint64_t current_time,
                                               std::optional<int32_t> thread_id) const {
  const TextBox* next_box = nullptr;
  uint64_t next_box_time = std::numeric_limits<uint64_t>::max();
  std::vector<std::shared_ptr<TimerChain>> chains = GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (const auto& block : *chain) {
      if (!block.Intersects(current_time, next_box_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TextBox& box = block[i];
        auto box_time = box.GetTimerInfo().end();
        if ((box.GetTimerInfo().function_id() == function_id) &&
            (!thread_id || thread_id.value() == box.GetTimerInfo().thread_id()) &&
            (box_time > current_time) && (next_box_time > box_time)) {
          next_box = &box;
          next_box_time = box_time;
        }
      }
    }
  }
  return next_box;
}

void TimeGraph::RequestUpdatePrimitives() {
  update_primitives_requested_ = true;
  RequestRedraw();
}

// UpdatePrimitives updates all the drawable track timers in the timegraph's batcher
void TimeGraph::UpdatePrimitives(PickingMode picking_mode) {
  ORBIT_SCOPE_FUNCTION;
  CHECK(app_->GetStringManager() != nullptr);

  batcher_.StartNewFrame();
  text_renderer_static_.Clear();

  if (capture_data_) {
    capture_min_timestamp_ =
        std::min(capture_min_timestamp_, capture_data_->GetCallstackData()->min_time());
    capture_max_timestamp_ =
        std::max(capture_max_timestamp_, capture_data_->GetCallstackData()->max_time());
  }

  time_window_us_ = max_time_us_ - min_time_us_;
  world_start_x_ = canvas_->GetWorldTopLeftX();
  world_width_ = canvas_->GetWorldWidth();
  uint64_t min_tick = GetTickFromUs(min_time_us_);
  uint64_t max_tick = GetTickFromUs(max_time_us_);

  track_manager_->SortTracks();
  track_manager_->UpdateMovingTrackSorting();
  track_manager_->UpdateTracks(&batcher_, min_tick, max_tick, picking_mode);
  // Coordinates from CaptureWindows could need an update if we modified the vertical size of some
  // track.
  GetCanvas()->UpdateWorldTopLeftY();

  update_primitives_requested_ = false;
}

void TimeGraph::SelectCallstacks(float world_start, float world_end, int32_t thread_id) {
  if (world_start > world_end) {
    std::swap(world_end, world_start);
  }

  uint64_t t0 = GetTickFromWorld(world_start);
  uint64_t t1 = GetTickFromWorld(world_end);

  CHECK(capture_data_);
  std::vector<CallstackEvent> selected_callstack_events =
      (thread_id == orbit_base::kAllProcessThreadsTid)
          ? capture_data_->GetCallstackData()->GetCallstackEventsInTimeRange(t0, t1)
          : capture_data_->GetCallstackData()->GetCallstackEventsOfTidInTimeRange(thread_id, t0,
                                                                                  t1);

  selected_callstack_events_per_thread_.clear();
  for (CallstackEvent& event : selected_callstack_events) {
    selected_callstack_events_per_thread_[event.thread_id()].emplace_back(event);
    selected_callstack_events_per_thread_[orbit_base::kAllProcessThreadsTid].emplace_back(event);
  }

  app_->SelectCallstackEvents(selected_callstack_events, thread_id);

  RequestUpdatePrimitives();
}

const std::vector<CallstackEvent>& TimeGraph::GetSelectedCallstackEvents(int32_t tid) {
  return selected_callstack_events_per_thread_[tid];
}

void TimeGraph::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  ORBIT_SCOPE("TimeGraph::Draw");
  current_mouse_time_ns_ = GetTickFromWorld(canvas_->GetMouseX());

  const bool picking = picking_mode != PickingMode::kNone;
  if ((!picking && update_primitives_requested_) || picking) {
    UpdatePrimitives(picking_mode);
  }

  DrawTracks(canvas, picking_mode);
  DrawOverlay(canvas, picking_mode);

  redraw_requested_ = false;
}

namespace {

[[nodiscard]] std::string GetLabelBetweenIterators(const InstrumentedFunction& function_a,
                                                   const InstrumentedFunction& function_b) {
  const std::string& function_from = function_a.function_name();
  const std::string& function_to = function_b.function_name();
  return absl::StrFormat("%s to %s", function_from, function_to);
}

std::string GetTimeString(const TimerInfo& timer_a, const TimerInfo& timer_b) {
  absl::Duration duration = TicksToDuration(timer_a.start(), timer_b.start());

  return GetPrettyTime(duration);
}

[[nodiscard]] Color GetIteratorBoxColor(uint64_t index) {
  constexpr uint64_t kNumColors = 2;
  const Color kLightBlueGray = Color(177, 203, 250, 60);
  const Color kMidBlueGray = Color(81, 102, 157, 60);
  Color colors[kNumColors] = {kLightBlueGray, kMidBlueGray};
  return colors[index % kNumColors];
}

}  // namespace

void TimeGraph::DrawIteratorBox(GlCanvas* canvas, Vec2 pos, Vec2 size, const Color& color,
                                const std::string& label, const std::string& time,
                                float text_box_y) {
  Box box(pos, size, GlCanvas::kZValueOverlay);
  canvas->GetBatcher()->AddBox(box, color);

  std::string text = absl::StrFormat("%s: %s", label, time);

  float max_size = size[0];

  const Color kBlack(0, 0, 0, 255);
  float text_width = canvas->GetTextRenderer().AddTextTrailingCharsPrioritized(
      text.c_str(), pos[0], text_box_y + layout_.GetTextOffset(), GlCanvas::kZValueTextUi, kBlack,
      time.length(), GetLayout().GetFontSize(), max_size);

  Vec2 white_box_size(std::min(static_cast<float>(text_width), max_size), GetTextBoxHeight());
  Vec2 white_box_position(pos[0], text_box_y);

  Box white_box(white_box_position, white_box_size, GlCanvas::kZValueOverlayTextBackground);

  const Color kWhite(255, 255, 255, 255);
  canvas->GetBatcher()->AddBox(white_box, kWhite);

  Vec2 line_from(pos[0] + white_box_size[0], white_box_position[1] + GetTextBoxHeight() / 2.f);
  Vec2 line_to(pos[0] + size[0], white_box_position[1] + GetTextBoxHeight() / 2.f);
  canvas->GetBatcher()->AddLine(line_from, line_to, GlCanvas::kZValueOverlay,
                                Color(255, 255, 255, 255));
}

void TimeGraph::DrawOverlay(GlCanvas* canvas, PickingMode picking_mode) {
  if (picking_mode != PickingMode::kNone || iterator_text_boxes_.empty()) {
    return;
  }

  std::vector<std::pair<uint64_t, const TextBox*>> boxes(iterator_text_boxes_.size());
  std::copy(iterator_text_boxes_.begin(), iterator_text_boxes_.end(), boxes.begin());

  // Sort boxes by start time.
  std::sort(boxes.begin(), boxes.end(),
            [](const std::pair<uint64_t, const TextBox*>& box_a,
               const std::pair<uint64_t, const TextBox*>& box_b) -> bool {
              return box_a.second->GetTimerInfo().start() < box_b.second->GetTimerInfo().start();
            });

  // We will need the world x coordinates for the timers multiple times, so
  // we avoid recomputing them and just cache them here.
  std::vector<float> x_coords;
  x_coords.reserve(boxes.size());

  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();

  float world_start_y = canvas->GetWorldTopLeftY();
  float world_height = canvas->GetWorldHeight();

  double inv_time_window = 1.0 / GetTimeWindowUs();

  // Draw lines for iterators.
  for (const auto& box : boxes) {
    const TimerInfo& timer_info = box.second->GetTimerInfo();

    double start_us = GetUsFromTick(timer_info.start());
    double normalized_start = start_us * inv_time_window;
    auto world_timer_x = static_cast<float>(world_start_x + normalized_start * world_width);

    Vec2 pos(world_timer_x, world_start_y);
    x_coords.push_back(pos[0]);

    canvas->GetBatcher()->AddVerticalLine(pos, -world_height, GlCanvas::kZValueOverlay,
                                          GetThreadColor(timer_info.thread_id()));
  }

  // Draw boxes with timings between iterators.
  for (size_t k = 1; k < boxes.size(); ++k) {
    Vec2 pos(x_coords[k - 1], world_start_y - world_height);
    float size_x = x_coords[k] - pos[0];
    Vec2 size(size_x, world_height);
    Color color = GetIteratorBoxColor(k - 1);

    uint64_t id_a = boxes[k - 1].first;
    uint64_t id_b = boxes[k].first;
    CHECK(iterator_id_to_function_id_.find(id_a) != iterator_id_to_function_id_.end());
    CHECK(iterator_id_to_function_id_.find(id_b) != iterator_id_to_function_id_.end());
    uint64_t function_a_id = iterator_id_to_function_id_.at(id_a);
    uint64_t function_b_id = iterator_id_to_function_id_.at(id_b);
    const CaptureData& capture_data = app_->GetCaptureData();
    const InstrumentedFunction* function_a =
        capture_data.GetInstrumentedFunctionById(function_a_id);
    const InstrumentedFunction* function_b =
        capture_data.GetInstrumentedFunctionById(function_b_id);
    CHECK(function_a != nullptr);
    CHECK(function_b != nullptr);
    const std::string& label = GetLabelBetweenIterators(*function_a, *function_b);
    const std::string& time =
        GetTimeString(boxes[k - 1].second->GetTimerInfo(), boxes[k].second->GetTimerInfo());

    // Distance from the bottom where we don't want to draw.
    float bottom_margin = layout_.GetBottomMargin();

    // The height of text is chosen such that the text of the last box drawn is
    // at pos[1] + bottom_margin (lowest possible position) and the height of
    // the box showing the overall time (see below) is at pos[1] + (world_height
    // / 2.f), corresponding to the case k == 0 in the formula for 'text_y'.
    float height_per_text = ((world_height / 2.f) - bottom_margin) /
                            static_cast<float>(iterator_text_boxes_.size() - 1);
    float text_y = pos[1] + (world_height / 2.f) - static_cast<float>(k) * height_per_text;

    DrawIteratorBox(canvas, pos, size, color, label, time, text_y);
  }

  // When we have at least 3 boxes, we also draw the total time from the first
  // to the last iterator.
  if (boxes.size() > 2) {
    size_t last_index = boxes.size() - 1;

    Vec2 pos(x_coords[0], world_start_y - world_height);
    float size_x = x_coords[last_index] - pos[0];
    Vec2 size(size_x, world_height);

    std::string time =
        GetTimeString(boxes[0].second->GetTimerInfo(), boxes[last_index].second->GetTimerInfo());
    std::string label("Total");

    float text_y = pos[1] + (world_height / 2.f);

    // We do not want the overall box to add any color, so we just set alpha to
    // 0.
    const Color kColorBlackTransparent(0, 0, 0, 0);
    DrawIteratorBox(canvas, pos, size, kColorBlackTransparent, label, time, text_y);
  }
}

std::string TimeGraph::GetThreadNameFromTid(uint32_t tid) {
  const std::string kEmptyString;
  return capture_data_ ? capture_data_->GetThreadName(tid) : kEmptyString;
}

void TimeGraph::DrawTracks(GlCanvas* canvas, PickingMode picking_mode) {
  for (auto& track : track_manager_->GetVisibleTracks()) {
    float z_offset = 0;
    if (track->IsPinned()) {
      z_offset = GlCanvas::kZOffsetPinnedTrack;
    } else if (track->IsMoving()) {
      z_offset = GlCanvas::kZOffsetMovingTack;
    }
    track->Draw(canvas, picking_mode, z_offset);
  }
}

void TimeGraph::SetThreadFilter(const std::string& filter) {
  track_manager_->SetFilter(filter);
  RequestUpdatePrimitives();
}

void TimeGraph::SelectAndZoom(const TextBox* text_box) {
  CHECK(text_box);
  Zoom(text_box->GetTimerInfo());
  Select(text_box);
}

void TimeGraph::JumpToNeighborBox(const TextBox* from, JumpDirection jump_direction,
                                  JumpScope jump_scope) {
  const TextBox* goal = nullptr;
  if (!from) {
    return;
  }
  auto function_id = from->GetTimerInfo().function_id();
  auto current_time = from->GetTimerInfo().end();
  auto thread_id = from->GetTimerInfo().thread_id();
  if (jump_direction == JumpDirection::kPrevious) {
    switch (jump_scope) {
      case JumpScope::kSameDepth:
        goal = FindPrevious(from);
        break;
      case JumpScope::kSameFunction:
        goal = FindPreviousFunctionCall(function_id, current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindPreviousFunctionCall(function_id, current_time, thread_id);
        break;
      default:
        // Other choices are not implemented.
        CHECK(false);
        break;
    }
  }
  if (jump_direction == JumpDirection::kNext) {
    switch (jump_scope) {
      case JumpScope::kSameDepth:
        goal = FindNext(from);
        break;
      case JumpScope::kSameFunction:
        goal = FindNextFunctionCall(function_id, current_time);
        break;
      case JumpScope::kSameThreadSameFunction:
        goal = FindNextFunctionCall(function_id, current_time, thread_id);
        break;
      default:
        CHECK(false);
        break;
    }
  }
  if (jump_direction == JumpDirection::kTop) {
    goal = FindTop(from);
  }
  if (jump_direction == JumpDirection::kDown) {
    goal = FindDown(from);
  }
  if (goal) {
    Select(goal);
  }
}

void TimeGraph::UpdateRightMargin(float margin) {
  {
    if (right_margin_ != margin) {
      right_margin_ = margin;
      RequestUpdatePrimitives();
    }
  }
}

const TextBox* TimeGraph::FindPrevious(const TextBox* from) {
  CHECK(from);
  const TimerInfo& timer_info = from->GetTimerInfo();
  if (timer_info.type() == TimerInfo::kGpuActivity) {
    return track_manager_->GetOrCreateGpuTrack(timer_info.timeline_hash())->GetLeft(from);
  }
  return track_manager_->GetOrCreateThreadTrack(timer_info.thread_id())->GetLeft(from);
}

const TextBox* TimeGraph::FindNext(const TextBox* from) {
  CHECK(from);
  const TimerInfo& timer_info = from->GetTimerInfo();
  if (timer_info.type() == TimerInfo::kGpuActivity) {
    return track_manager_->GetOrCreateGpuTrack(timer_info.timeline_hash())->GetRight(from);
  }
  return track_manager_->GetOrCreateThreadTrack(timer_info.thread_id())->GetRight(from);
}

const TextBox* TimeGraph::FindTop(const TextBox* from) {
  CHECK(from);
  const TimerInfo& timer_info = from->GetTimerInfo();
  if (timer_info.type() == TimerInfo::kGpuActivity) {
    return track_manager_->GetOrCreateGpuTrack(timer_info.timeline_hash())->GetUp(from);
  }
  return track_manager_->GetOrCreateThreadTrack(timer_info.thread_id())->GetUp(from);
}

const TextBox* TimeGraph::FindDown(const TextBox* from) {
  CHECK(from);
  const TimerInfo& timer_info = from->GetTimerInfo();
  if (timer_info.type() == TimerInfo::kGpuActivity) {
    return track_manager_->GetOrCreateGpuTrack(timer_info.timeline_hash())->GetDown(from);
  }
  return track_manager_->GetOrCreateThreadTrack(timer_info.thread_id())->GetDown(from);
}

void TimeGraph::DrawText(GlCanvas* canvas, float layer) {
  if (draw_text_) {
    text_renderer_static_.RenderLayer(canvas->GetBatcher(), layer);
  }
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

bool TimeGraph::HasFrameTrack(uint64_t function_id) const {
  auto frame_tracks = track_manager_->GetFrameTracks();
  return (std::find_if(frame_tracks.begin(), frame_tracks.end(), [&](auto frame_track) {
            return frame_track->GetFunctionId() == function_id;
          }) != frame_tracks.end());
}

void TimeGraph::RemoveFrameTrack(uint64_t function_id) {
  track_manager_->RemoveFrameTrack(function_id);
  RequestUpdatePrimitives();
}
