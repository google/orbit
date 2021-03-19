// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTrack.h"

#include <GteVector.h>
#include <absl/time/time.h>

#include <algorithm>
#include <memory>
#include <optional>

#include "App.h"
#include "Batcher.h"
#include "CoreUtils.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitClientModel/CaptureData.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "TriangleToggle.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::TimerInfo;

constexpr const char* kSwQueueString = "sw queue";
constexpr const char* kHwQueueString = "hw queue";
constexpr const char* kHwExecutionString = "hw execution";
constexpr const char* kCmdBufferString = "command buffer";

namespace orbit_gl {

std::string MapGpuTimelineToTrackLabel(std::string_view timeline) {
  std::string queue_pretty_name;
  std::string timeline_label(timeline);
  if (timeline.rfind("gfx", 0) == 0) {
    queue_pretty_name = "Graphics queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("sdma", 0) == 0) {
    queue_pretty_name = "DMA queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("comp", 0) == 0) {
    queue_pretty_name = "Compute queue";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  if (timeline.rfind("vce", 0) == 0) {
    queue_pretty_name = "Video Coding Engine";
    timeline_label = absl::StrFormat(" (%s)", timeline);
  }
  std::string debug_marker_label;
  if (timeline.find("_marker") != std::string::npos) {
    // Space at start is intentional as we want that space between the queue name
    // and "debug markers". This makes constructing the string simpler.
    debug_marker_label = " debug markers";
  }
  return absl::StrFormat("%s%s%s", queue_pretty_name, debug_marker_label, timeline_label);
}

}  // namespace orbit_gl

GpuTrack::GpuTrack(TimeGraph* time_graph, TimeGraphLayout* layout, uint64_t timeline_hash,
                   OrbitApp* app, const CaptureData* capture_data)
    : TimerTrack(time_graph, layout, app, capture_data) {
  text_renderer_ = time_graph->GetTextRenderer();
  timeline_hash_ = timeline_hash;
  string_manager_ = app->GetStringManager();

  std::string timeline =
      app_->GetStringManager()->Get(timeline_hash).value_or(std::to_string(timeline_hash));
  std::string label = orbit_gl::MapGpuTimelineToTrackLabel(timeline);
  SetName(timeline);
  SetLabel(label);
  // This min combine two cases, label == timeline and when label includes timeline
  SetNumberOfPrioritizedTrailingCharacters(std::min(label.size(), timeline.size() + 2));

  // Gpu tracks are collapsed by default.
  collapse_toggle_->SetState(TriangleToggle::State::kCollapsed,
                             TriangleToggle::InitialStateUpdate::kReplaceInitialState);
}

void GpuTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  // In case of having command buffer timers, we need to double the depth of the GPU timers (as we
  // are drawing the corresponding command buffer timers below them). Therefore, we watch out for
  // those timers.
  if (timer_info.type() == TimerInfo::kGpuCommandBuffer) {
    has_vulkan_layer_command_buffer_timers_ = true;
  }
  TimerTrack::OnTimer(timer_info);
}

bool GpuTrack::IsTimerActive(const TimerInfo& timer_info) const {
  bool is_same_tid_as_selected = timer_info.thread_id() == app_->selected_thread_id();
  // We do not properly track the PID for GPU jobs and we still want to show
  // all jobs as active when no thread is selected, so this logic is a bit
  // different than SchedulerTrack::IsTimerActive.
  bool no_thread_selected = app_->selected_thread_id() == orbit_base::kAllProcessThreadsTid;

  return is_same_tid_as_selected || no_thread_selected;
}

Color GpuTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected,
                              bool is_highlighted) const {
  const Color kInactiveColor(100, 100, 100, 255);
  const Color kSelectionColor(0, 128, 255, 255);
  if (is_highlighted) {
    return TimerTrack::kHighlightColor;
  }
  if (is_selected) {
    return kSelectionColor;
  }
  if (!IsTimerActive(timer_info)) {
    return kInactiveColor;
  }
  if (timer_info.has_color()) {
    CHECK(timer_info.color().red() < 256);
    CHECK(timer_info.color().green() < 256);
    CHECK(timer_info.color().blue() < 256);
    CHECK(timer_info.color().alpha() < 256);
    return Color(static_cast<uint8_t>(timer_info.color().red()),
                 static_cast<uint8_t>(timer_info.color().green()),
                 static_cast<uint8_t>(timer_info.color().blue()),
                 static_cast<uint8_t>(timer_info.color().alpha()));
  }
  if (timer_info.type() == TimerInfo::kGpuDebugMarker) {
    std::string marker_text = string_manager_->Get(timer_info.user_data_key()).value_or("");
    return TimeGraph::GetColor(marker_text);
  }

  // We color code the timeslices for GPU activity using the color
  // of the CPU thread track that submitted the job.
  Color color = TimeGraph::GetThreadColor(timer_info.thread_id());

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

float GpuTrack::GetYFromTimer(const TimerInfo& timer_info) const {
  auto adjusted_depth = static_cast<float>(timer_info.depth());
  if (collapse_toggle_->IsCollapsed()) {
    adjusted_depth = 0.f;
  }
  if (timer_info.type() == TimerInfo::kGpuDebugMarker) {
    return pos_[1] - layout_->GetTextBoxHeight() * (adjusted_depth + 1.f);
  }
  CHECK(timer_info.type() == TimerInfo::kGpuActivity ||
        timer_info.type() == TimerInfo::kGpuCommandBuffer);

  // Command buffer timers are drawn underneath the matching "hw execution" timer, which has the
  // same depth value as the command buffer timer. Therefore, we need to double the depth in the
  // case that we have command buffer timers.
  if (has_vulkan_layer_command_buffer_timers_) {
    adjusted_depth *= 2.f;
  }

  float gap_space = adjusted_depth * layout_->GetSpaceBetweenGpuDepths();

  // Command buffer timers have the same depth value as their matching "hw execution" timer.
  // As we want to draw command buffers underneath the hw execution timers, we need to increase
  // the depth by one.
  if (timer_info.type() == TimerInfo::kGpuCommandBuffer) {
    adjusted_depth += 1.f;
  }
  return pos_[1] - layout_->GetTextBoxHeight() * (adjusted_depth + 1.f) - gap_space;
}

// When track is collapsed, only draw "hardware execution" timers and the root "debug markers".
bool GpuTrack::TimerFilter(const TimerInfo& timer_info) const {
  if (collapse_toggle_->IsCollapsed()) {
    std::string gpu_stage = string_manager_->Get(timer_info.user_data_key()).value_or("");
    return gpu_stage == kHwExecutionString ||
           (timer_info.type() == TimerInfo::kGpuDebugMarker && timer_info.depth() == 0);
  }
  return true;
}

void GpuTrack::SetTimesliceText(const TimerInfo& timer_info, float min_x, float z_offset,
                                TextBox* text_box) {
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Nanoseconds(timer_info.end() - timer_info.start()));

    text_box->SetElapsedTimeTextLength(time.length());

    CHECK(timer_info.type() == TimerInfo::kGpuActivity ||
          timer_info.type() == TimerInfo::kGpuCommandBuffer ||
          timer_info.type() == TimerInfo::kGpuDebugMarker);

    std::string text = absl::StrFormat(
        "%s  %s", string_manager_->Get(timer_info.user_data_key()).value_or(""), time.c_str());
    text_box->SetText(text);
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout_->GetTextOffset(),
      GlCanvas::kZValueBox + z_offset, kTextWhite, text_box->GetElapsedTimeTextLength(),
      layout_->CalculateZoomedFontSize(), max_size);
}

std::string GpuTrack::GetTooltip() const {
  return "Shows scheduling and execution times for selected GPU job "
         "submissions";
}

float GpuTrack::GetHeight() const {
  bool collapsed = collapse_toggle_->IsCollapsed();
  uint32_t depth = collapsed ? 1 : GetDepth();
  uint32_t num_gaps = depth > 0 ? depth - 1 : 0;
  if (has_vulkan_layer_command_buffer_timers_ && !collapsed) {
    depth *= 2;
  }
  return layout_->GetTextBoxHeight() * depth + (num_gaps * layout_->GetSpaceBetweenGpuDepths()) +
         layout_->GetTrackBottomMargin();
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

std::string GpuTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const TextBox* text_box = batcher.GetTextBox(id);
  if (!text_box || text_box->GetTimerInfo().type() == TimerInfo::kCoreActivity) {
    return "";
  }

  std::string gpu_stage =
      string_manager_->Get(text_box->GetTimerInfo().user_data_key()).value_or("");
  if (gpu_stage == kSwQueueString) {
    return GetSwQueueTooltip(text_box->GetTimerInfo());
  }
  if (gpu_stage == kHwQueueString) {
    return GetHwQueueTooltip(text_box->GetTimerInfo());
  }
  if (gpu_stage == kHwExecutionString) {
    return GetHwExecutionTooltip(text_box->GetTimerInfo());
  }
  if (gpu_stage == kCmdBufferString) {
    return GetCommandBufferTooltip(text_box->GetTimerInfo());
  }
  if (text_box->GetTimerInfo().type() == TimerInfo::kGpuDebugMarker) {
    return GetDebugMarkerTooltip(text_box->GetTimerInfo());
  }

  return "";
}

std::string GpuTrack::GetSwQueueTooltip(const TimerInfo& timer_info) const {
  CHECK(capture_data_ != nullptr);
  return absl::StrFormat(
      "<b>Software Queue</b><br/>"
      "<i>Time between amdgpu_cs_ioctl (job submitted) and "
      "amdgpu_sched_run_job (job scheduled)</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      capture_data_->GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetHwQueueTooltip(const TimerInfo& timer_info) const {
  CHECK(capture_data_ != nullptr);
  return absl::StrFormat(
      "<b>Hardware Queue</b><br/><i>Time between amdgpu_sched_run_job "
      "(job scheduled) and start of GPU execution</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      capture_data_->GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetHwExecutionTooltip(const TimerInfo& timer_info) const {
  CHECK(capture_data_ != nullptr);
  return absl::StrFormat(
      "<b>Harware Execution</b><br/>"
      "<i>End is marked by \"dma_fence_signaled\" event for this command "
      "buffer submission</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      capture_data_->GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetCommandBufferTooltip(
    const orbit_client_protos::TimerInfo& timer_info) const {
  return absl::StrFormat(
      "<b>Command Buffer Execution</b><br/>"
      "<i>At `vkBeginCommandBuffer` and `vkEndCommandBuffer` `vkCmdWriteTimestamp`s have been "
      "inserted. The GPU timestamps get aligned with the corresponding hardware execution of the "
      "submission.</i>"
      "<br/>"
      "<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      app_->GetCaptureData().GetThreadName(timer_info.thread_id()), timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}

std::string GpuTrack::GetDebugMarkerTooltip(
    const orbit_client_protos::TimerInfo& timer_info) const {
  std::string marker_text = string_manager_->Get(timer_info.user_data_key()).value_or("");
  return absl::StrFormat(
      "<b>Vulkan Debug Marker</b><br/>"
      "<i>At the marker's begin and end `vkCmdWriteTimestamp`s have been "
      "inserted. The GPU timestamps get aligned with the corresponding hardware execution of the "
      "submission.</i>"
      "<br/>"
      "<br/>"
      "<b>Marker text:</b> %s<br/>"
      "<b>Submitted from thread:</b> %s [%d]<br/>"
      "<b>Time:</b> %s",
      marker_text, app_->GetCaptureData().GetThreadName(timer_info.thread_id()),
      timer_info.thread_id(),
      GetPrettyTime(TicksToDuration(timer_info.start(), timer_info.end())).c_str());
}
