// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraph.h"

#include <OrbitBase/Logging.h>

#include <algorithm>
#include <utility>

#include "App.h"
#include "Batcher.h"
#include "Capture.h"
#include "EventTracer.h"
#include "EventTrack.h"
#include "FunctionUtils.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "Log.h"
#include "Params.h"
#include "PickingManager.h"
#include "SamplingProfiler.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "Utils.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

TimeGraph* GCurrentTimeGraph = nullptr;

//-----------------------------------------------------------------------------
TimeGraph::TimeGraph() {
  m_LastThreadReorder.Start();
  scheduler_track_ = GetOrCreateSchedulerTrack();

  // The process track is a special ThreadTrack of id "0".
  process_track_ = GetOrCreateThreadTrack(0);
}

//-----------------------------------------------------------------------------
Color TimeGraph::GetThreadColor(ThreadID tid) const {
  static unsigned char a = 255;
  static std::vector<Color> s_ThreadColors{
      Color(231, 68, 53, a),    // red
      Color(43, 145, 175, a),   // blue
      Color(185, 117, 181, a),  // purple
      Color(87, 166, 74, a),    // green
      Color(215, 171, 105, a),  // beige
      Color(248, 101, 22, a)    // orange
  };
  return s_ThreadColors[tid % s_ThreadColors.size()];
}

//-----------------------------------------------------------------------------
void TimeGraph::SetStringManager(std::shared_ptr<StringManager> str_manager) {
  string_manager_ = std::move(str_manager);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetCanvas(GlCanvas* a_Canvas) {
  m_Canvas = a_Canvas;
  m_TextRenderer->SetCanvas(a_Canvas);
  m_TextRendererStatic.SetCanvas(a_Canvas);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetFontSize(int a_FontSize) {
  m_TextRenderer->SetFontSize(a_FontSize);
  m_TextRendererStatic.SetFontSize(a_FontSize);
}

//-----------------------------------------------------------------------------
void TimeGraph::Clear() {
  m_Batcher.Reset();
  capture_min_timestamp_ = std::numeric_limits<TickType>::max();
  capture_max_timestamp_ = 0;
  m_ThreadCountMap.clear();
  GEventTracer.GetEventBuffer().Reset();

  ScopeLock lock(m_Mutex);
  tracks_.clear();
  scheduler_track_ = nullptr;
  thread_tracks_.clear();
  gpu_tracks_.clear();

  cores_seen_.clear();
  scheduler_track_ = GetOrCreateSchedulerTrack();

  // The process track is a special ThreadTrack of id "0".
  process_track_ = GetOrCreateThreadTrack(0);
}

//-----------------------------------------------------------------------------
double GNumHistorySeconds = 2.f;

//-----------------------------------------------------------------------------
bool TimeGraph::UpdateCaptureMinMaxTimestamps() {
  capture_min_timestamp_ = std::numeric_limits<TickType>::max();

  m_Mutex.lock();
  for (auto& track : tracks_) {
    if (track->GetNumTimers()) {
      TickType min = track->GetMinTime();
      if (min > 0 && min < capture_min_timestamp_) {
        capture_min_timestamp_ = min;
      }
    }
  }
  m_Mutex.unlock();

  if (GEventTracer.GetEventBuffer().HasEvent()) {
    capture_min_timestamp_ = std::min(
        capture_min_timestamp_, GEventTracer.GetEventBuffer().GetMinTime());
    capture_max_timestamp_ = std::max(
        capture_max_timestamp_, GEventTracer.GetEventBuffer().GetMaxTime());
  }

  return capture_min_timestamp_ != std::numeric_limits<TickType>::max();
}

//-----------------------------------------------------------------------------
void TimeGraph::ZoomAll() {
  if (UpdateCaptureMinMaxTimestamps()) {
    m_MaxTimeUs =
        MicroSecondsFromTicks(capture_min_timestamp_, capture_max_timestamp_);
    m_MinTimeUs = m_MaxTimeUs - (GNumHistorySeconds * 1000 * 1000);
    if (m_MinTimeUs < 0) m_MinTimeUs = 0;

    NeedsUpdate();
  }
}

void TimeGraph::Zoom(TickType min, TickType max) {
  double start = MicroSecondsFromTicks(capture_min_timestamp_, min);
  double end = MicroSecondsFromTicks(capture_min_timestamp_, max);

  double mid = start + ((end - start) / 2.0);
  double extent = 1.1 * (end - start) / 2.0;

  SetMinMax(mid - extent, mid + extent);
}

//-----------------------------------------------------------------------------
void TimeGraph::Zoom(const TextBox* a_TextBox) {
  const Timer& timer = a_TextBox->GetTimer();
  Zoom(timer.m_Start, timer.m_End);
}

//-----------------------------------------------------------------------------
double TimeGraph::GetCaptureTimeSpanUs() {
  if (UpdateCaptureMinMaxTimestamps()) {
    return MicroSecondsFromTicks(capture_min_timestamp_,
                                 capture_max_timestamp_);
  }

  return 0;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetCurrentTimeSpanUs() { return m_MaxTimeUs - m_MinTimeUs; }

//-----------------------------------------------------------------------------
void TimeGraph::ZoomTime(float a_ZoomValue, double a_MouseRatio) {
  m_ZoomValue = a_ZoomValue;
  m_MouseRatio = a_MouseRatio;

  static double incrementRatio = 0.1;
  double scale = a_ZoomValue > 0 ? 1 + incrementRatio : 1 - incrementRatio;

  double CurrentTimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_RefTimeUs = m_MinTimeUs + a_MouseRatio * CurrentTimeWindowUs;

  double timeLeft = std::max(m_RefTimeUs - m_MinTimeUs, 0.0);
  double timeRight = std::max(m_MaxTimeUs - m_RefTimeUs, 0.0);

  double minTimeUs = m_RefTimeUs - scale * timeLeft;
  double maxTimeUs = m_RefTimeUs + scale * timeRight;

  if (maxTimeUs - minTimeUs < 0.001 /*1 ns*/) {
    return;
  }

  SetMinMax(minTimeUs, maxTimeUs);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetMinMax(double a_MinTimeUs, double a_MaxTimeUs) {
  double desiredTimeWindow = a_MaxTimeUs - a_MinTimeUs;
  m_MinTimeUs = std::max(a_MinTimeUs, 0.0);
  m_MaxTimeUs =
      std::min(m_MinTimeUs + desiredTimeWindow, GetCaptureTimeSpanUs());

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::PanTime(int a_InitialX, int a_CurrentX, int a_Width,
                        double a_InitialTime) {
  m_TimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  double initialLocalTime =
      static_cast<double>(a_InitialX) / a_Width * m_TimeWindowUs;
  double dt =
      static_cast<double>(a_CurrentX - a_InitialX) / a_Width * m_TimeWindowUs;
  double currentTime = a_InitialTime - dt;
  m_MinTimeUs = clamp(currentTime - initialLocalTime, 0.0,
                      GetCaptureTimeSpanUs() - m_TimeWindowUs);
  m_MaxTimeUs = m_MinTimeUs + m_TimeWindowUs;

  NeedsUpdate();
}

void TimeGraph::HorizontallyMoveIntoView(VisibilityType vis_type, TickType min,
                                         TickType max, double distance) {
  if (IsVisible(vis_type, min, max)) {
    return;
  }

  double start = MicroSecondsFromTicks(capture_min_timestamp_, min);
  double end = MicroSecondsFromTicks(capture_min_timestamp_, max);

  double CurrentTimeWindowUs = m_MaxTimeUs - m_MinTimeUs;

  if (vis_type == VisibilityType::kFullyVisible && CurrentTimeWindowUs < (end - start)){
    Zoom(min, max);
    return;
  }

  double mid = start + ((end - start) / 2.0);

  // Mirror the final center position if we have to move left
  if (start < m_MinTimeUs) {
    distance = 1 - distance;
  }

  SetMinMax(mid - CurrentTimeWindowUs * (1 - distance),
            mid + CurrentTimeWindowUs * distance);

  NeedsUpdate();
}

void TimeGraph::HorizontallyMoveIntoView(VisibilityType vis_type,
                                         const TextBox* text_box,
                                         double distance) {
  HorizontallyMoveIntoView(vis_type, text_box->GetTimer().m_Start,
                           text_box->GetTimer().m_End, distance);
}

void TimeGraph::VerticallyMoveIntoView(const TextBox* text_box) {
  CHECK(text_box != nullptr);
  const Timer& timer = text_box->GetTimer();
  auto thread_track = GetOrCreateThreadTrack(timer.m_TID);
  auto text_box_y_position = thread_track->GetYFromDepth(timer.m_Depth);

  float world_top_left_y = m_Canvas->GetWorldTopLeftY();
  float min_world_top_left_y =
      text_box_y_position + m_Layout.GetSpaceBetweenTracks() + m_Layout.GetTopMargin();
  float max_world_top_left_y = text_box_y_position + m_Canvas->GetWorldHeight() -
                              GetTextBoxHeight() - m_Layout.GetBottomMargin();
  CHECK (min_world_top_left_y <= max_world_top_left_y);
  world_top_left_y = std::min(world_top_left_y, max_world_top_left_y);
  world_top_left_y = std::max(world_top_left_y, min_world_top_left_y);
  m_Canvas->SetWorldTopLeftY(world_top_left_y);
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::OnDrag(float a_Ratio) {
  double timeSpan = GetCaptureTimeSpanUs();
  double timeWindow = m_MaxTimeUs - m_MinTimeUs;
  m_MinTimeUs = a_Ratio * (timeSpan - timeWindow);
  m_MaxTimeUs = m_MinTimeUs + timeWindow;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTime(double a_Ratio) {
  double CurrentWidth = m_MaxTimeUs - m_MinTimeUs;
  double Delta = a_Ratio * CurrentWidth;
  return m_MinTimeUs + Delta;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTimeIntervalMicro(double a_Ratio) {
  double CurrentWidth = m_MaxTimeUs - m_MinTimeUs;
  return a_Ratio * CurrentWidth;
}

//-----------------------------------------------------------------------------
uint64_t TimeGraph::GetGpuTimelineHash(const Timer& timer) const {
  return timer.m_UserData[1];
}

//-----------------------------------------------------------------------------
void TimeGraph::ProcessTimer(const Timer& a_Timer) {
  if (a_Timer.m_End > capture_max_timestamp_) {
    capture_max_timestamp_ = a_Timer.m_End;
  }

  switch (a_Timer.m_Type) {
    case Timer::CORE_ACTIVITY:
      Capture::GHasContextSwitches = true;
      break;
    default:
      break;
  }

  if (a_Timer.m_FunctionAddress > 0) {
    Function* func = Capture::GTargetProcess->GetFunctionFromAddress(
        a_Timer.m_FunctionAddress);
    if (func != nullptr) {
      ++Capture::GFunctionCountMap[a_Timer.m_FunctionAddress];
      FunctionUtils::UpdateStats(func, a_Timer);
    }
  }

  if (a_Timer.m_Type == Timer::GPU_ACTIVITY) {
    uint64_t timeline_hash = GetGpuTimelineHash(a_Timer);
    std::shared_ptr<GpuTrack> track = GetOrCreateGpuTrack(timeline_hash);
    track->OnTimer(a_Timer);
  } else {
    std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(a_Timer.m_TID);
    if (a_Timer.m_Type == Timer::INTROSPECTION) {
      const Color kGreenIntrospection(87, 166, 74, 255);
      track->SetColor(kGreenIntrospection);
    }

    if (a_Timer.m_Type != Timer::CORE_ACTIVITY) {
      track->OnTimer(a_Timer);
      ++m_ThreadCountMap[a_Timer.m_TID];
    } else {
      scheduler_track_->OnTimer(a_Timer);
      cores_seen_.insert(a_Timer.m_Processor);
    }
  }
}

//-----------------------------------------------------------------------------
uint32_t TimeGraph::GetNumTimers() const {
  uint32_t numTimers = 0;
  ScopeLock lock(m_Mutex);
  for (const auto& track : tracks_) {
    numTimers += track->GetNumTimers();
  }
  return numTimers;
}

//-----------------------------------------------------------------------------
uint32_t TimeGraph::GetNumCores() const {
  ScopeLock lock(m_Mutex);
  return cores_seen_.size();
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> TimeGraph::GetAllTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& track : tracks_) {
    Append(chains, track->GetAllChains());
  }
  return chains;
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>>
TimeGraph::GetAllThreadTrackTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& [_, track] : thread_tracks_) {
    Append(chains, track->GetAllChains());
  }
  return chains;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateMaxTimeStamp(TickType a_Time) {
  if (a_Time > capture_max_timestamp_) {
    capture_max_timestamp_ = a_Time;
  }
};

//-----------------------------------------------------------------------------
float TimeGraph::GetThreadTotalHeight() { return std::abs(min_y_); }

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromTick(TickType a_Time) const {
  if (m_TimeWindowUs > 0) {
    double start =
        MicroSecondsFromTicks(capture_min_timestamp_, a_Time) - m_MinTimeUs;
    double normalizedStart = start / m_TimeWindowUs;
    float pos = float(m_WorldStartX + normalizedStart * m_WorldWidth);
    return pos;
  }

  return 0;
}

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromUs(double a_Micros) const {
  return GetWorldFromTick(GetTickFromUs(a_Micros));
}

//-----------------------------------------------------------------------------
double TimeGraph::GetUsFromTick(TickType time) const {
  return MicroSecondsFromTicks(capture_min_timestamp_, time) - m_MinTimeUs;
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetTickFromWorld(float a_WorldX) {
  double ratio =
      m_WorldWidth != 0
          ? static_cast<double>((a_WorldX - m_WorldStartX) / m_WorldWidth)
          : 0;
  double timeStamp = GetTime(ratio);

  return capture_min_timestamp_ + TicksFromMicroseconds(timeStamp);
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetTickFromUs(double a_MicroSeconds) const {
  return capture_min_timestamp_ + TicksFromMicroseconds(a_MicroSeconds);
}

//-----------------------------------------------------------------------------
void TimeGraph::GetWorldMinMax(float& a_Min, float& a_Max) const {
  a_Min = GetWorldFromTick(capture_min_timestamp_);
  a_Max = GetWorldFromTick(capture_max_timestamp_);
}

void TimeGraph::Select(const TextBox* a_TextBox) {
  TextBox* text_box = const_cast<TextBox*>(a_TextBox);
  Capture::GSelectedTextBox = text_box;
  HorizontallyMoveIntoView(VisibilityType::kPartlyVisible, a_TextBox);
  VerticallyMoveIntoView(a_TextBox);
}

const TextBox* TimeGraph::FindPreviousFunctionCall(
    uint64_t function_address, TickType current_time) const {
  TextBox* previous_box = nullptr;
  TickType previous_box_time = std::numeric_limits<TickType>::lowest();
  std::vector<std::shared_ptr<TimerChain>> chains =
      GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      if (!block.Intersects(previous_box_time, current_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        TextBox& box = block[i];
        auto box_time = box.GetTimer().m_End;
        if ((box.GetTimer().m_FunctionAddress == function_address) &&
            (box_time < current_time) && (previous_box_time < box_time)) {
          previous_box = &box;
          previous_box_time = box_time;
        }
      }
    }
  }
  return previous_box;
}

const TextBox* TimeGraph::FindNextFunctionCall(uint64_t function_address,
                                               TickType current_time) const {
  TextBox* next_box = nullptr;
  TickType next_box_time = std::numeric_limits<TickType>::max();
  std::vector<std::shared_ptr<TimerChain>> chains =
      GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      if (!block.Intersects(current_time, next_box_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        TextBox& box = block[i];
        auto box_time = box.GetTimer().m_End;
        if ((box.GetTimer().m_FunctionAddress == function_address) &&
            (box_time > current_time) && (next_box_time > box_time)) {
          next_box = &box;
          next_box_time = box_time;
        }
      }
    }
  }
  return next_box;
}

//-----------------------------------------------------------------------------
void TimeGraph::NeedsUpdate() {
  m_NeedsUpdatePrimitives = true;
  // If the primitives need to be updated, we also have to redraw.
  m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdatePrimitives(PickingMode picking_mode) {
  CHECK(string_manager_);

  m_Batcher.Reset();
  m_TextRendererStatic.Clear();

  UpdateMaxTimeStamp(GEventTracer.GetEventBuffer().GetMaxTime());

  m_SceneBox = m_Canvas->GetSceneBox();
  m_TimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_WorldStartX = m_Canvas->GetWorldTopLeftX();
  m_WorldWidth = m_Canvas->GetWorldWidth();
  TickType min_tick = GetTickFromUs(m_MinTimeUs);
  TickType max_tick = GetTickFromUs(m_MaxTimeUs);

  SortTracks();

  float current_y = -m_Layout.GetSchedulerTrackOffset();

  for (auto& track : sorted_tracks_) {
    track->SetY(current_y);
    track->UpdatePrimitives(min_tick, max_tick, picking_mode);
    current_y -= (track->GetHeight() + m_Layout.GetSpaceBetweenTracks());
  }

  min_y_ = current_y;
  m_NeedsUpdatePrimitives = false;
}

//-----------------------------------------------------------------------------
std::vector<CallstackEvent> TimeGraph::SelectEvents(float a_WorldStart,
                                                    float a_WorldEnd,
                                                    ThreadID a_TID) {
  if (a_WorldStart > a_WorldEnd) {
    std::swap(a_WorldEnd, a_WorldStart);
  }

  TickType t0 = GetTickFromWorld(a_WorldStart);
  TickType t1 = GetTickFromWorld(a_WorldEnd);

  std::vector<CallstackEvent> selected_callstack_events =
      GEventTracer.GetEventBuffer().GetCallstackEvents(t0, t1, a_TID);

  selected_callstack_events_per_thread_.clear();
  for (CallstackEvent& event : selected_callstack_events) {
    selected_callstack_events_per_thread_[event.m_TID].emplace_back(event);
    selected_callstack_events_per_thread_[0].emplace_back(event);
  }

  // Generate selection report.
  std::shared_ptr<SamplingProfiler> samplingProfiler =
      std::make_shared<SamplingProfiler>(Capture::GTargetProcess);

  samplingProfiler->SetGenerateSummary(a_TID == 0);

  for (CallstackEvent& event : selected_callstack_events) {
    if (Capture::GSamplingProfiler->GetCallStack(event.m_Id)) {
      samplingProfiler->AddCallStack(event);
    }
  }
  samplingProfiler->ProcessSamples();

  if (samplingProfiler->GetNumSamples() > 0) {
    GOrbitApp->AddSelectionReport(samplingProfiler);
  }

  NeedsUpdate();

  return selected_callstack_events;
}

const std::vector<CallstackEvent>& TimeGraph::GetSelectedCallstackEvents(
    ThreadID tid) {
  return selected_callstack_events_per_thread_[tid];
}

//-----------------------------------------------------------------------------
void TimeGraph::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  const bool picking = picking_mode != PickingMode::kNone;
  if ((!picking && m_NeedsUpdatePrimitives) || picking) {
    UpdatePrimitives(picking_mode);
  }

  DrawTracks(canvas, picking_mode);
  DrawOverlay(canvas, picking_mode);
  m_Batcher.Draw(picking);

  m_NeedsRedraw = false;
}

namespace {

std::string GetLabelBetweenIterators(const TextBox* box_a,
                                     const TextBox* box_b) {
  std::string function_from =
      Capture::GAddressToFunctionName[box_a->GetTimer().m_FunctionAddress];
  std::string function_to =
      Capture::GAddressToFunctionName[box_b->GetTimer().m_FunctionAddress];
  return absl::StrFormat("%s to %s", function_from, function_to);
}

std::string GetTimeString(const TextBox* box_a, const TextBox* box_b) {
  double micros = MicroSecondsFromTicks(box_a->GetTimer().m_Start,
                                        box_b->GetTimer().m_Start);

  return GetPrettyTime(micros * 0.001);
}

Color GetIteratorBoxColor(uint64_t index) {
  constexpr uint64_t kNumColors = 2;
  const Color kLightBlueGray = Color(177, 203, 250, 60);
  const Color kMidBlueGray = Color(81, 102, 157, 60);
  Color colors[kNumColors] = {kLightBlueGray, kMidBlueGray};
  return colors[index % kNumColors];
}

void DrawIteratorBox(GlCanvas* canvas, Vec2 pos, Vec2 size, const Color& color,
                     const std::string& label, const std::string& time,
                     float text_y) {
  Box box(pos, size, GlCanvas::Z_VALUE_OVERLAY);
  canvas->GetBatcher()->AddBox(box, color, PickingID::BOX);

  std::string text = absl::StrFormat("%s: %s", label, time);

  constexpr const float kAdditionalSpaceForLine = 10.f;
  constexpr const float kLeftOffset = 5.f;

  float max_size = size[0];
  canvas->GetTextRenderer().AddTextTrailingCharsPrioritized(
      text.c_str(), pos[0] + kLeftOffset, text_y + kAdditionalSpaceForLine,
      GlCanvas::Z_VALUE_TEXT, Color(255, 255, 255, 255), time.length(),
      max_size);

  constexpr const float kOffsetBelowText = kAdditionalSpaceForLine / 2.f;
  Vec2 line_from(pos[0], text_y + kOffsetBelowText);
  Vec2 line_to(pos[0] + size[0], text_y + kOffsetBelowText);
  canvas->GetBatcher()->AddLine(line_from, line_to, GlCanvas::Z_VALUE_OVERLAY,
                                Color(255, 255, 255, 255), PickingID::LINE);
}

}  // namespace

void TimeGraph::DrawOverlay(GlCanvas* canvas, PickingMode picking_mode) {
  if (picking_mode != PickingMode::kNone || 
      overlay_current_textboxes_.size() == 0) {
    return;
  }

  std::vector<std::pair<uint64_t, const TextBox*>> boxes(
      overlay_current_textboxes_.size());
  std::copy(overlay_current_textboxes_.begin(),
            overlay_current_textboxes_.end(), boxes.begin());

  // Sort boxes by start time.
  std::sort(boxes.begin(), boxes.end(),
            [](const std::pair<uint64_t, const TextBox*>& box_a,
               const std::pair<uint64_t, const TextBox*>& box_b) -> bool {
              return box_a.second->GetTimer().m_Start <
                     box_b.second->GetTimer().m_Start;
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
    const Timer& timer = box.second->GetTimer();

    double start_us = GetUsFromTick(timer.m_Start);
    double normalized_start = start_us * inv_time_window;
    float world_timer_x =
        static_cast<float>(world_start_x + normalized_start * world_width);

    Vec2 pos(world_timer_x, world_start_y);
    x_coords.push_back(pos[0]);

    canvas->GetBatcher()->AddVerticalLine(
        pos, -world_height, GlCanvas::Z_VALUE_OVERLAY,
        GetThreadColor(timer.m_TID), PickingID::LINE, nullptr);
  }

  // Draw boxes with timings between iterators.
  for (size_t k = 1; k < boxes.size(); ++k) {
    Vec2 pos(x_coords[k - 1], world_start_y - world_height);
    float size_x = x_coords[k] - pos[0];
    Vec2 size(size_x, world_height);
    Color color = GetIteratorBoxColor(k - 1);

    const std::string& label =
        GetLabelBetweenIterators(boxes[k - 1].second, boxes[k].second);
    const std::string& time =
        GetTimeString(boxes[k - 1].second, boxes[k].second);

    // Distance from the bottom where we don't want to draw.
    float bottom_margin = m_Layout.GetBottomMargin();

    // The height of text is chosen such that the text of the last box drawn is
    // at pos[1] + bottom_margin (lowest possible position) and the height of
    // the box showing the overall time (see below) is at pos[1] + (world_height
    // / 2.f), corresponding to the case k == 0 in the formula for 'text_y'.
    float height_per_text =
        ((world_height / 2.f) - bottom_margin) /
        static_cast<float>(overlay_current_textboxes_.size() - 1);
    float text_y =
        pos[1] + (world_height / 2.f) - static_cast<float>(k) * height_per_text;

    DrawIteratorBox(canvas, pos, size, color, label, time, text_y);
  }

  // When we have at least 3 boxes, we also draw the total time from the first
  // to the last iterator.
  if (boxes.size() > 2) {
    size_t last_index = boxes.size() - 1;

    Vec2 pos(x_coords[0], world_start_y - world_height);
    float size_x = x_coords[last_index] - pos[0];
    Vec2 size(size_x, world_height);

    std::string time = GetTimeString(boxes[0].second, boxes[last_index].second);
    std::string label("Total");

    float text_y = pos[1] + (world_height / 2.f);

    // We do not want the overall box to add any color, so we just set alpha to
    // 0.
    const Color kColorBlackTransparent(0, 0, 0, 0);
    DrawIteratorBox(canvas, pos, size, kColorBlackTransparent, label, time,
                    text_y);
  }
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawTracks(GlCanvas* canvas, PickingMode picking_mode) {
  uint32_t num_cores = GetNumCores();
  m_Layout.SetNumCores(num_cores);
  scheduler_track_->SetLabel(
      absl::StrFormat("Scheduler (%u cores)", num_cores));
  for (auto& track : sorted_tracks_) {
    if (track->GetType() == Track::kThreadTrack) {
      auto thread_track = std::static_pointer_cast<ThreadTrack>(track);
      int32_t tid = thread_track->GetThreadId();
      if (tid == 0) {
        // This is the process_track_.
        std::string process_name = Capture::GProcessName;
        thread_track->SetName(process_name);
        thread_track->SetLabel(process_name + " (all threads)");
      } else {
        const std::string& thread_name = Capture::GThreadNames[tid];
        track->SetName(thread_name);
        std::string track_label = absl::StrFormat("%s [%u]", thread_name, tid);
        track->SetLabel(track_label);
      }
    }

    track->Draw(canvas, picking_mode);
  }
}

//-----------------------------------------------------------------------------
std::shared_ptr<SchedulerTrack> TimeGraph::GetOrCreateSchedulerTrack() {
  ScopeLock lock(m_Mutex);
  std::shared_ptr<SchedulerTrack> track = scheduler_track_;
  if (track == nullptr) {
    track = std::make_shared<SchedulerTrack>(this);
    tracks_.emplace_back(track);
    scheduler_track_ = track;
  }
  return track;
}

std::shared_ptr<ThreadTrack> TimeGraph::GetOrCreateThreadTrack(ThreadID a_TID) {
  ScopeLock lock(m_Mutex);
  std::shared_ptr<ThreadTrack> track = thread_tracks_[a_TID];
  if (track == nullptr) {
    track = std::make_shared<ThreadTrack>(this, a_TID);
    tracks_.emplace_back(track);
    thread_tracks_[a_TID] = track;
    track->SetEventTrackColor(GetThreadColor(a_TID));
  }
  return track;
}

std::shared_ptr<GpuTrack> TimeGraph::GetOrCreateGpuTrack(
    uint64_t timeline_hash) {
  ScopeLock lock(m_Mutex);
  std::shared_ptr<GpuTrack> track = gpu_tracks_[timeline_hash];
  if (track == nullptr) {
    track = std::make_shared<GpuTrack>(this, string_manager_, timeline_hash);
    std::string timeline = string_manager_->Get(timeline_hash).value_or("");
    std::string label = OrbitGl::MapGpuTimelineToTrackLabel(timeline);
    track->SetName(timeline);
    track->SetLabel(label);
    tracks_.emplace_back(track);
    gpu_tracks_[timeline_hash] = track;
  }

  return track;
}

//-----------------------------------------------------------------------------
void TimeGraph::SetThreadFilter(const std::string& a_Filter) {
  m_ThreadFilter = a_Filter;
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::SortTracks() {
  // Get or create thread track from events' thread id.
  {
    ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
    m_EventCount.clear();

    for (auto& pair : GEventTracer.GetEventBuffer().GetCallstacks()) {
      ThreadID threadID = pair.first;
      std::map<uint64_t, CallstackEvent>& callstacks = pair.second;
      m_EventCount[threadID] = callstacks.size();
      GetOrCreateThreadTrack(threadID);
    }
  }

  // Reorder threads once every second when capturing
  if (!Capture::IsCapturing() || m_LastThreadReorder.QueryMillis() > 1000.0) {
    std::vector<ThreadID> sortedThreadIds;

    // Show threads with instrumented functions first
    std::vector<std::pair<ThreadID, uint32_t>> sortedThreads =
        OrbitUtils::ReverseValueSort(m_ThreadCountMap);
    for (auto& pair : sortedThreads) {
      // Track "0" holds all target process sampling info, it is handled
      // separately.
      if (pair.first != 0) sortedThreadIds.push_back(pair.first);
    }

    // Then show threads sorted by number of events
    std::vector<std::pair<ThreadID, uint32_t>> sortedByEvents =
        OrbitUtils::ReverseValueSort(m_EventCount);
    for (auto& pair : sortedByEvents) {
      // Track "0" holds all target process sampling info, it is handled
      // separately.
      if (pair.first == 0) continue;
      if (m_ThreadCountMap.find(pair.first) == m_ThreadCountMap.end()) {
        sortedThreadIds.push_back(pair.first);
      }
    }

    // Filter thread ids if needed
    if (!m_ThreadFilter.empty()) {
      std::vector<std::string> filters = absl::StrSplit(m_ThreadFilter, ' ');
      std::vector<ThreadID> filteredThreadIds;
      for (ThreadID tid : sortedThreadIds) {
        std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(tid);

        for (auto& filter : filters) {
          if (track && absl::StrContains(track->GetName(), filter)) {
            filteredThreadIds.push_back(tid);
          }
        }
      }
      sortedThreadIds = filteredThreadIds;
    }

    sorted_tracks_.clear();

    // Scheduler Track.
    if (!scheduler_track_->IsEmpty()) {
      sorted_tracks_.emplace_back(scheduler_track_);
    }

    // Gpu Tracks.
    for (const auto& timeline_and_track : gpu_tracks_) {
      sorted_tracks_.emplace_back(timeline_and_track.second);
    }

    // Process Track.
    if (!process_track_->IsEmpty()) {
      sorted_tracks_.emplace_back(process_track_);
    }

    // Thread Tracks.
    for (auto thread_id : sortedThreadIds) {
      std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(thread_id);
      if (!track->IsEmpty()) {
        sorted_tracks_.emplace_back(track);
      }
    }

    m_LastThreadReorder.Reset();
  }
}

void TimeGraph::SelectAndZoom(const TextBox* text_box) {
  CHECK(text_box);
  Zoom(text_box);
  Select(text_box);
}

void TimeGraph::JumpToNeighborBox(TextBox* from, JumpDirection jump_direction,
                                  JumpScope jump_scope) {
  const TextBox* goal = nullptr;
  if (!from) {
    return;
  }
  auto function_address = from->GetTimer().m_FunctionAddress;
  auto current_time = from->GetTimer().m_End;
  if (jump_direction == JumpDirection::kPrevious) {
    if (jump_scope == JumpScope::kSameThread) {
      goal = FindPrevious(from);
    }
    if (jump_scope == JumpScope::kSameFunction) {
      goal = FindPreviousFunctionCall(function_address, current_time);
    }
  }
  if (jump_direction == JumpDirection::kNext) {
    if (jump_scope == JumpScope::kSameThread) {
      goal = FindNext(from);
    }
    if (jump_scope == JumpScope::kSameFunction) {
      goal = FindNextFunctionCall(function_address, current_time);
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

const TextBox* TimeGraph::FindPrevious(TextBox* from) {
  CHECK(from);
  const Timer& timer = from->GetTimer();
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    return GetOrCreateGpuTrack(GetGpuTimelineHash(timer))->GetLeft(from);
  } else {
    return GetOrCreateThreadTrack(timer.m_TID)->GetLeft(from);
  }
  return nullptr;
}

const TextBox* TimeGraph::FindNext(TextBox* from) {
  CHECK(from);
  const Timer& timer = from->GetTimer();
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    return GetOrCreateGpuTrack(GetGpuTimelineHash(timer))->GetRight(from);
  } else {
    return GetOrCreateThreadTrack(timer.m_TID)->GetRight(from);
  }
  return nullptr;
}

const TextBox* TimeGraph::FindTop(TextBox* from) {
  CHECK(from);
  const Timer& timer = from->GetTimer();
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    return GetOrCreateGpuTrack(GetGpuTimelineHash(timer))->GetUp(from);
  } else {
    return GetOrCreateThreadTrack(timer.m_TID)->GetUp(from);
  }
  return nullptr;
}

const TextBox* TimeGraph::FindDown(TextBox* from) {
  CHECK(from);
  const Timer& timer = from->GetTimer();
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    return GetOrCreateGpuTrack(GetGpuTimelineHash(timer))->GetDown(from);
  } else {
    return GetOrCreateThreadTrack(timer.m_TID)->GetDown(from);
  }
  return nullptr;
}

//----------------------------------------------------------------------------
void TimeGraph::DrawText(GlCanvas* canvas) {
  if (m_DrawText) {
    m_TextRendererStatic.Display(canvas->GetBatcher());
  }
}

bool TimeGraph::IsFullyVisible(TickType min, TickType max) const {
  double start = MicroSecondsFromTicks(capture_min_timestamp_, min);
  double end = MicroSecondsFromTicks(capture_min_timestamp_, max);

  return start > m_MinTimeUs && end < m_MaxTimeUs;
}

bool TimeGraph::IsPartlyVisible(TickType min, TickType max) const {
  double start = MicroSecondsFromTicks(capture_min_timestamp_, min);
  double end = MicroSecondsFromTicks(capture_min_timestamp_, max);

  double startUs = m_MinTimeUs;

  if (startUs > end || m_MaxTimeUs < start) {
    return false;
  }

  return true;
}

bool TimeGraph::IsVisible(VisibilityType vis_type, TickType min, TickType max) const {
  switch(vis_type) {
    case VisibilityType::kPartlyVisible:
      return IsPartlyVisible(min, max);
    case VisibilityType::kFullyVisible:
      return IsFullyVisible(min, max);
    default:
      return false;   
  }
}
