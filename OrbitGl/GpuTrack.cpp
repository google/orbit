// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTrack.h"

#include "App.h"
#include "Capture.h"
#include "GlCanvas.h"
#include "Profiling.h"
#include "TimeGraph.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::TimerInfo;

constexpr const char* kSwQueueString = "sw queue";
constexpr const char* kHwQueueString = "hw queue";
constexpr const char* kHwExecutionString = "hw execution";

namespace OrbitGl {

std::string MapGpuTimelineToTrackLabel(std::string_view timeline) {
  std::string label;
  if (timeline.rfind("gfx", 0) == 0) {
    return absl::StrFormat("Graphics queue (%s)", timeline);
  } else if (timeline.rfind("sdma", 0) == 0) {
    return absl::StrFormat("Transfer queue (%s)", timeline);
  } else if (timeline.rfind("comp", 0) == 0) {
    return absl::StrFormat("Compute queue (%s)", timeline);
  } else {
    // On AMD, this should not happen and we don't support tracepoints for
    // other GPUs (at the moment). We return the timeline to make sure we
    // at least display something. When we add support for other GPU
    // tracepoints, this needs to be changed.
    return std::string(timeline);
  }
}

}  // namespace OrbitGl

GpuTrack::GpuTrack(TimeGraph* time_graph, std::shared_ptr<StringManager> string_manager,
                   uint64_t timeline_hash)
    : TimerTrack(time_graph) {
  text_renderer_ = time_graph->GetTextRenderer();
  timeline_hash_ = timeline_hash;

  num_timers_ = 0;
  min_time_ = std::numeric_limits<TickType>::max();
  max_time_ = std::numeric_limits<TickType>::min();

  string_manager_ = string_manager;

  // Gpu tracks are collapsed by default.
  collapse_toggle_->SetState(TriangleToggle::State::kCollapsed,
                             TriangleToggle::InitialStateUpdate::kReplaceInitialState);
}

bool GpuTrack::IsTimerActive(const TimerInfo& timer_info) const {
  bool is_same_tid_as_selected = timer_info.thread_id() == GOrbitApp->selected_thread_id();
  // We do not properly track the PID for GPU jobs and we still want to show
  // all jobs as active when no thread is selected, so this logic is a bit
  // different than SchedulerTrack::IsTimerActive.
  bool no_thread_selected = GOrbitApp->selected_thread_id() == -1;

  return is_same_tid_as_selected || no_thread_selected;
}

Color GpuTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected) const {
  const Color kInactiveColor(100, 100, 100, 255);
  const Color kSelectionColor(0, 128, 255, 255);
  if (is_selected) {
    return kSelectionColor;
  } else if (!IsTimerActive(timer_info)) {
    return kInactiveColor;
  }

  // We color code the timeslices for GPU activity using the color
  // of the CPU thread track that submitted the job.
  Color color = time_graph_->GetThreadColor(timer_info.thread_id());

  // We disambiguate the different types of GPU activity based on the
  // string that is displayed on their timeslice.
  float coeff = 1.0f;
  std::string gpu_stage = string_manager_->Get(timer_info.user_data_key()).value_or("");
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
  if (!(timer_info.depth() & 0x1)) {
    color[3] = kOddAlpha;
  }

  return color;
}

float GpuTrack::GetYFromDepth(uint32_t depth) const {
  float adjusted_depth = static_cast<float>(depth);
  if (collapse_toggle_->IsCollapsed()) {
    adjusted_depth = 0.f;
  }
  return m_Pos[1] - time_graph_->GetLayout().GetTextBoxHeight() * (adjusted_depth + 1.f);
}

// When track is collapsed, only draw "hardware execution" timers.
bool GpuTrack::TimerFilter(const TimerInfo& timer_info) const {
  if (collapse_toggle_->IsCollapsed()) {
    std::string gpu_stage = string_manager_->Get(timer_info.user_data_key()).value_or("");
    if (gpu_stage != kHwExecutionString) {
      return false;
    }
  }
  return true;
}

void GpuTrack::SetTimesliceText(const TimerInfo& timer_info, double elapsed_us, float min_x,
                                TextBox* text_box) {
  TimeGraphLayout layout = time_graph_->GetLayout();
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Microseconds(elapsed_us));

    text_box->SetElapsedTimeTextLength(time.length());

    CHECK(timer_info.type() == TimerInfo::kGpuActivity);

    std::string text = absl::StrFormat(
        "%s  %s", time_graph_->GetStringManager()->Get(timer_info.user_data_key()).value_or(""),
        time.c_str());
    text_box->SetText(text);
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x, text_box->GetPosY() + layout.GetTextOffset(),
      GlCanvas::Z_VALUE_TEXT, kTextWhite, text_box->GetElapsedTimeTextLength(), max_size);
}

std::string GpuTrack::GetTooltip() const {
  return "Shows scheduling and execution times for selected GPU job "
         "submissions";
}

float GpuTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  bool collapsed = collapse_toggle_->IsCollapsed();
  uint32_t depth = collapsed ? 1 : GetDepth();
  return layout.GetTextBoxHeight() * depth + layout.GetTrackBottomMargin();
}

const TextBox* GpuTrack::GetLeft(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  uint64_t timeline_hash = timer_info.user_data_key();
  if (timeline_hash == timeline_hash_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer_info.depth());
    if (timers) return timers->GetElementBefore(text_box);
  }
  return nullptr;
}

const TextBox* GpuTrack::GetRight(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  uint64_t timeline_hash = timer_info.user_data_key();
  if (timeline_hash == timeline_hash_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer_info.depth());
    if (timers) return timers->GetElementAfter(text_box);
  }
  return nullptr;
}

std::string GpuTrack::GetBoxTooltip(PickingId id) const {
  const TextBox* text_box = time_graph_->GetBatcher().GetTextBox(id);
  if (!text_box || text_box->GetTimerInfo().type() == TimerInfo::kCoreActivity) {
    return "";
  }

  std::string gpu_stage =
      string_manager_->Get(text_box->GetTimerInfo().user_data_key()).value_or("");
  if (gpu_stage == kSwQueueString) {
    return GetSwQueueTooltip(text_box->GetTimerInfo());
  } else if (gpu_stage == kHwQueueString) {
    return GetHwQueueTooltip(text_box->GetTimerInfo());
  } else if (gpu_stage == kHwExecutionString) {
    return GetHwExecutionTooltip(text_box->GetTimerInfo());
  }

  return "";
}

std::string GpuTrack::GetSwQueueTooltip(const TimerInfo& timer_info) const {
  return absl::StrFormat(
      "<b>Software Queue</b><br/>"
      "<i>Time between amdgpu_cs_ioctl (job submitted) and "
      "amdgpu_sched_run_job (job scheduled)</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      Capture::capture_data_.GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetHwQueueTooltip(const TimerInfo& timer_info) const {
  return absl::StrFormat(
      "<b>Hardware Queue</b><br/><i>Time between amdgpu_sched_run_job "
      "(job scheduled) and start of GPU execution</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      Capture::capture_data_.GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetHwExecutionTooltip(const TimerInfo& timer_info) const {
  return absl::StrFormat(
      "<b>Harware Execution</b><br/>"
      "<i>End is marked by \"dma_fence_signaled\" event for this command "
      "buffer submission</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      Capture::capture_data_.GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}
