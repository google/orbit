// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SchedulerTrack.h"

#include "Capture.h"
#include "EventTrack.h"
#include "FunctionUtils.h"
#include "GlCanvas.h"
#include "TextBox.h"
#include "TimeGraph.h"

using orbit_client_protos::TimerInfo;

const Color kInactiveColor(100, 100, 100, 255);
const Color kSelectionColor(0, 128, 255, 255);

SchedulerTrack::SchedulerTrack(TimeGraph* time_graph) : TimerTrack(time_graph) {}

float SchedulerTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  uint32_t num_gaps = depth_ > 0 ? depth_ - 1 : 0;
  return (depth_ * layout.GetTextCoresHeight()) + (num_gaps * layout.GetSpaceBetweenCores()) +
         layout.GetTrackBottomMargin();
}

bool SchedulerTrack::IsTimerActive(const TimerInfo& timer_info) const {
  bool is_same_tid_as_selected = timer_info.thread_id() == Capture::GSelectedThreadId;
  int32_t capture_process_id = Capture::capture_data_.process_id();
  bool is_same_pid_as_target =
      capture_process_id == 0 || capture_process_id == timer_info.process_id();

  return is_same_tid_as_selected || (Capture::GSelectedThreadId == 0 && is_same_pid_as_target);
}

Color SchedulerTrack::GetTimerColor(const TimerInfo& timer_info, bool is_selected) const {
  if (is_selected) {
    return kSelectionColor;
  } else if (!IsTimerActive(timer_info)) {
    return kInactiveColor;
  }
  return time_graph_->GetThreadColor(timer_info.thread_id());
}

float SchedulerTrack::GetYFromDepth(uint32_t depth) const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  uint32_t num_gaps = depth;
  return m_Pos[1] - (layout.GetTextCoresHeight() * (depth + 1)) -
         num_gaps * layout.GetSpaceBetweenCores();
}

std::string SchedulerTrack::GetTooltip() const {
  return "Shows scheduling information for CPU cores";
}

void SchedulerTrack::UpdateBoxHeight() {
  box_height_ = time_graph_->GetLayout().GetTextCoresHeight();
}

std::string SchedulerTrack::GetBoxTooltip(PickingId id) const {
  TextBox* text_box = time_graph_->GetBatcher().GetTextBox(id);
  if (!text_box) {
    return "";
  }

  return absl::StrFormat(
      "<b>CPU Core activity</b><br/>"
      "<br/>"
      "<b>Core:</b> %d<br/>"
      "<b>Thread:</b> %s [%d]<br/>",
      text_box->GetTimerInfo().processor(),
      Capture::capture_data_.GetThreadName(text_box->GetTimerInfo().thread_id()),
      text_box->GetTimerInfo().thread_id());
}
