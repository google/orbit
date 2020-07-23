// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTrack.h"

#include <limits>

#include "Capture.h"
#include "GlCanvas.h"
#include "TimeGraph.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

constexpr const char* kSwQueueString = "sw queue";
constexpr const char* kHwQueueString = "hw queue";
constexpr const char* kHwExecutionString = "hw execution";

//-----------------------------------------------------------------------------
GpuTrack::GpuTrack(TimeGraph* time_graph,
                   std::shared_ptr<StringManager> string_manager,
                   uint64_t timeline_hash)
    : Track(time_graph) {
  text_renderer_ = time_graph->GetTextRenderer();
  timeline_hash_ = timeline_hash;

  num_timers_ = 0;
  min_time_ = std::numeric_limits<TickType>::max();
  max_time_ = std::numeric_limits<TickType>::min();

  string_manager_ = string_manager;

  // Gpu tracks are collapsed by default.
  collapse_toggle_.SetState(TriangleToggle::State::kCollapsed);
}

//-----------------------------------------------------------------------------
void GpuTrack::Draw(GlCanvas* canvas, bool picking) {
  float track_height = GetHeight();
  float track_width = canvas->GetWorldWidth();

  SetPos(canvas->GetWorldTopLeftX(), m_Pos[1]);
  SetSize(track_width, track_height);

  Track::Draw(canvas, picking);
}

//-----------------------------------------------------------------------------
Color GpuTrack::GetTimerColor(const Timer& timer, bool is_selected,
                              bool inactive) const {
  const Color kInactiveColor(100, 100, 100, 255);
  const Color kSelectionColor(0, 128, 255, 255);
  if (is_selected) {
    return kSelectionColor;
  } else if (inactive) {
    return kInactiveColor;
  }

  // We color code the timeslices for GPU activity using the color
  // of the CPU thread track that submitted the job.
  Color color = time_graph_->GetThreadColor(timer.m_TID);

  // We disambiguate the different types of GPU activity based on the
  // string that is displayed on their timeslice.
  float coeff = 1.0f;
  std::string gpu_stage =
      string_manager_->Get(timer.m_UserData[0]).value_or("");
  if (gpu_stage == kSwQueueString) {
    coeff = 0.5f;
  } else if (gpu_stage == kHwQueueString) {
    coeff = 0.75f;
  } else if (gpu_stage == kHwExecutionString) {
    coeff = 1.0f;
  }

  color[0] = static_cast<uint8_t>(coeff * color[0]);
  color[1] = static_cast<uint8_t>(coeff * color[1]);
  color[2] = static_cast<uint8_t>(coeff * color[2]);

  constexpr uint8_t kOddAlpha = 210;
  if (!(timer.m_Depth & 0x1)) {
    color[3] = kOddAlpha;
  }

  return color;
}

//-----------------------------------------------------------------------------
inline float GetYFromDepth(const TimeGraphLayout& layout, float track_y,
                           uint32_t depth) {
  return track_y - layout.GetTextBoxHeight() * (depth + 1);
}

//-----------------------------------------------------------------------------
void GpuTrack::SetTimesliceText(const Timer& timer, double elapsed_us,
                                float min_x, TextBox* text_box) {
  TimeGraphLayout layout = time_graph_->GetLayout();
  if (text_box->GetText().empty()) {
    double elapsed_millis = elapsed_us * 0.001;
    std::string time = GetPrettyTime(elapsed_millis);

    text_box->SetElapsedTimeTextLength(time.length());

    CHECK(timer.m_Type == Timer::GPU_ACTIVITY);

    std::string text = absl::StrFormat(
        "%s; submitter: %d  %s",
        time_graph_->GetStringManager()->Get(timer.m_UserData[0]).value_or(""),
        timer.m_TID, time.c_str());
    text_box->SetText(text);
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x,
      text_box->GetPosY() + layout.GetTextOffset(), GlCanvas::Z_VALUE_TEXT,
      kTextWhite, text_box->GetElapsedTimeTextLength(), max_size);
}

//-----------------------------------------------------------------------------
void GpuTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
  Batcher* batcher = &time_graph_->GetBatcher();
  GlCanvas* canvas = time_graph_->GetCanvas();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const TextBox& scene_box = canvas->GetSceneBox();

  float min_x = scene_box.GetPosX();
  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();
  double inv_time_window = 1.0 / time_graph_->GetTimeWindowUs();
  bool is_collapsed = collapse_toggle_.IsCollapsed() && (depth_ > 1);

  std::vector<std::shared_ptr<TimerChain>> chains_by_depth = GetTimers();

  // We minimize overdraw when drawing lines for small events by discarding
  // events that would just draw over an already drawn line. When zoomed in
  // enough that all events are drawn as boxes, this has no effect. When zoomed
  // out, many events will be discarded quickly.
  uint64_t min_ignore = std::numeric_limits<uint64_t>::max();
  uint64_t max_ignore = std::numeric_limits<uint64_t>::min();

  uint64_t pixel_delta_in_ticks = static_cast<uint64_t>(TicksFromMicroseconds(
                                      time_graph_->GetTimeWindowUs())) /
                                  canvas->getWidth();
  uint64_t min_timegraph_tick =
      time_graph_->GetTickFromUs(time_graph_->GetMinTimeUs());

  for (auto& chain : chains_by_depth) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      if (!block.Intersects(min_tick, max_tick)) continue;
      // We have to reset this when we go to the next depth, as otherwise we
      // would miss drawing events that should be drawn.
      min_ignore = std::numeric_limits<uint64_t>::max();
      max_ignore = std::numeric_limits<uint64_t>::min();

      for (uint32_t k = 0; k < block.size(); ++k) {
        TextBox& text_box = block[k];
        const Timer& timer = text_box.GetTimer();
        if (min_tick > timer.m_End || max_tick < timer.m_Start) continue;
        if (timer.m_Start >= min_ignore && timer.m_End <= max_ignore) continue;

        UpdateDepth(timer.m_Depth + 1);
        double start_us = time_graph_->GetUsFromTick(timer.m_Start);
        double end_us = time_graph_->GetUsFromTick(timer.m_End);
        double elapsed_us = end_us - start_us;
        double normalized_start = start_us * inv_time_window;
        double normalized_length = elapsed_us * inv_time_window;
        float world_timer_width =
            static_cast<float>(normalized_length * world_width);
        float world_timer_x =
            static_cast<float>(world_start_x + normalized_start * world_width);
        uint8_t timer_depth = is_collapsed ? 0 : timer.m_Depth;
        float world_timer_y = GetYFromDepth(layout, m_Pos[1], timer_depth);

        bool is_visible_width = normalized_length * canvas->getWidth() > 1;
        bool is_selected = &text_box == Capture::GSelectedTextBox;

        Vec2 pos(world_timer_x, world_timer_y);
        Vec2 size(world_timer_width, layout.GetTextBoxHeight());
        float z = GlCanvas::Z_VALUE_BOX_ACTIVE;
        Color color = GetTimerColor(timer, is_selected, false);
        text_box.SetPos(pos);
        text_box.SetSize(size);

        // When track is collapsed, only draw "hardware execution" timers.
        if (is_collapsed) {
          std::string gpu_stage =
              string_manager_->Get(timer.m_UserData[0]).value_or("");
          if (gpu_stage != kHwExecutionString) {
            continue;
          }
        }

        if (is_visible_width) {
          if (!is_collapsed) {
            SetTimesliceText(timer, elapsed_us, min_x, &text_box);
          }
          batcher->AddShadedBox(pos, size, z, color, PickingID::BOX,
              std::make_shared<PickingUserData>(&text_box, [&](PickingID id) {
                return this->GetBoxTooltip(id);
              }));
        } else {
          auto type = PickingID::LINE;
          batcher->AddVerticalLine(
              pos, size[1], z, color, type,
              std::make_shared<PickingUserData>(&text_box, [&](PickingID id) {
                return this->GetBoxTooltip(id);
              }));
          // For lines, we can ignore the entire pixel into which this event
          // falls. We align this precisely on the pixel x-coordinate of the
          // current line being drawn (in ticks).
          min_ignore =
              min_timegraph_tick +
              ((timer.m_Start - min_timegraph_tick) / pixel_delta_in_ticks) *
                  pixel_delta_in_ticks;
          max_ignore = min_ignore + pixel_delta_in_ticks;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void GpuTrack::OnDrag(int x, int y) { Track::OnDrag(x, y); }

//-----------------------------------------------------------------------------
void GpuTrack::OnTimer(const Timer& timer) {
  TextBox text_box(Vec2(0, 0), Vec2(0, 0), "", Color(255, 0, 0, 255));
  text_box.SetTimer(timer);

  std::shared_ptr<TimerChain> timer_chain = timers_[timer.m_Depth];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<TimerChain>();
    timers_[timer.m_Depth] = timer_chain;
  }
  timer_chain->push_back(text_box);
  ++num_timers_;
  if (timer.m_Start < min_time_) min_time_ = timer.m_Start;
  if (timer.m_End > max_time_) max_time_ = timer.m_End;
}

std::string GpuTrack::GetTooltip() const {
  return "Shows scheduling and execution times for selected GPU calls.";
}

//-----------------------------------------------------------------------------
float GpuTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  bool collapsed = collapse_toggle_.IsCollapsed();
  uint32_t depth = collapsed ? 1 : GetDepth();
  return layout.GetTextBoxHeight() * depth + layout.GetTrackBottomMargin();
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> GpuTrack::GetTimers() {
  std::vector<std::shared_ptr<TimerChain>> timers;
  ScopeLock lock(mutex_);
  for (auto& pair : timers_) {
    timers.push_back(pair.second);
  }
  return timers;
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetFirstAfterTime(TickType time,
                                           uint32_t depth) const {
  std::shared_ptr<TimerChain> chain = GetTimers(depth);
  if (chain == nullptr) return nullptr;

  // TODO: do better than linear search...
  for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
    for (uint32_t k = 0; k < it->size(); ++k) {
      const TextBox& text_box = (*it)[k];
      if (text_box.GetTimer().m_Start > time) {
        return &text_box;
      }
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetFirstBeforeTime(TickType time,
                                            uint32_t depth) const {
  std::shared_ptr<TimerChain> chain = GetTimers(depth);
  if (chain == nullptr) return nullptr;

  const TextBox* text_box = nullptr;

  // TODO: do better than linear search...
  for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
    for (uint32_t k = 0; k < it->size(); ++k) {
      const TextBox& box = (*it)[k];
      if (box.GetTimer().m_Start > time) {
        return text_box;
      }
      text_box = &box;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<TimerChain> GpuTrack::GetTimers(uint32_t depth) const {
  ScopeLock lock(mutex_);
  auto it = timers_.find(depth);
  if (it != timers_.end()) return it->second;
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetLeft(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  uint64_t timeline_hash = timer.m_UserData[0];
  if (timeline_hash == timeline_hash_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementBefore(text_box);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetRight(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  uint64_t timeline_hash = timer.m_UserData[0];
  if (timeline_hash == timeline_hash_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementAfter(text_box);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetUp(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  return GetFirstBeforeTime(timer.m_Start, timer.m_Depth - 1);
}

//-----------------------------------------------------------------------------
const TextBox* GpuTrack::GetDown(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  return GetFirstAfterTime(timer.m_Start, timer.m_Depth + 1);
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> GpuTrack::GetAllChains() {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& pair : timers_) {
    chains.push_back(pair.second);
  }
  return chains;
}

//-----------------------------------------------------------------------------
std::string GpuTrack::GetBoxTooltip(PickingID id) const {
  TextBox* textBox = time_graph_->GetBatcher().GetTextBox(id);
  if (textBox) {
    if (textBox->GetTimer().m_Type != Timer::CORE_ACTIVITY) {
      std::string gpu_stage =
          string_manager_->Get(textBox->GetTimer().m_UserData[0]).value_or("");
      if (gpu_stage == kSwQueueString) {
        return GetSwQueueTooltip(textBox->GetTimer());
      } else if (gpu_stage == kHwQueueString) {
        return GetHwQueueTooltip(textBox->GetTimer());
      } else if (gpu_stage == kHwExecutionString) {
        return GetHwExecutionTooltip(textBox->GetTimer());
      }
    }
  }

  return "";
}

std::string GpuTrack::GetSwQueueTooltip(const Timer& timer) const { 
  return absl::StrFormat(
    "<b>Software Queue</b><br/>"
    "<i>Time between amdgpu_cs_ioctl and amdgpu_sched_run_job.</i>"
    "<br/>"
    "<br/>"
    "<b>Submitter thread:</b> %s [%d]<br/>"
    "<b>Time:</b> %s",
    Capture::GThreadNames[timer.m_TID].c_str(),
    timer.m_TID,
    GetPrettyTime(timer.ElapsedMillis()).c_str());
}

std::string GpuTrack::GetHwQueueTooltip(const Timer& timer) const {
  return absl::StrFormat("<b>Hardware Queue</b><br/><i>Time between amdgpu_sched_run_job and start of GPU execution</i>"
    "<br/>"
    "<br/>"
    "<b>Time:</b> %s",
    GetPrettyTime(timer.ElapsedMillis()).c_str());
}

std::string GpuTrack::GetHwExecutionTooltip(const Timer& timer) const {
  return absl::StrFormat("<b>Harware Execution</b><br/>"
    "<i>End is marked by \"dma_fence_signaled\" event for this command buffer submission</i>"
    "<br/>"
    "<br/>"
    "<b>Time:</b> %s",
    GetPrettyTime(timer.ElapsedMillis()).c_str());
}
