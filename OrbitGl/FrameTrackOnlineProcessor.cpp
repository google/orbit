// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FrameTrackOnlineProcessor.h"

#include <limits>

#include "CaptureData.h"
#include "TimeGraph.h"
#include "UserDefinedCaptureData.h"

FrameTrackOnlineProcessor::FrameTrackOnlineProcessor(
    const CaptureData& capture_data, const UserDefinedCaptureData& user_defined_capture_data,
    TimeGraph* time_graph)
    : time_graph_(time_graph) {
  for (const auto& function : user_defined_capture_data.frame_track_functions()) {
    const uint64_t function_address = capture_data.GetAbsoluteAddress(function);
    current_frame_track_functions_.insert(function_address);
    previous_timestamp_ns_.insert(
        std::make_pair(function_address, std::numeric_limits<uint64_t>::max()));
  }
}

void FrameTrackOnlineProcessor::ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                                             const orbit_client_protos::FunctionInfo& function) {
  uint64_t function_address = timer_info.function_address();
  if (!current_frame_track_functions_.contains(function_address)) {
    return;
  }
  uint64_t previous_timestamp_ns = previous_timestamp_ns_.at(function_address);
  if (previous_timestamp_ns == std::numeric_limits<uint64_t>::max()) {
    previous_timestamp_ns_[function_address] = timer_info.start();
    return;
  }

  if (previous_timestamp_ns < timer_info.start()) {
    orbit_client_protos::TimerInfo frame_timer;

    // TID is meaningless for this timer (start and end can be on two different threads).
    constexpr const int32_t kUnusedThreadId = -1;
    frame_timer.set_thread_id(kUnusedThreadId);
    frame_timer.set_start(previous_timestamp_ns);
    frame_timer.set_end(timer_info.start());
    // We use user_data_key to keep track of the frame number.
    frame_timer.set_user_data_key(current_frame_index_++);
    frame_timer.set_type(orbit_client_protos::TimerInfo::kFrame);

    time_graph_->ProcessTimer(frame_timer, &function);

    previous_timestamp_ns_[function_address] = timer_info.start();
  }
}