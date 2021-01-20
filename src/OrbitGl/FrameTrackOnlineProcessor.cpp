// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FrameTrackOnlineProcessor.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <limits>
#include <utility>

#include "OrbitClientData/FunctionInfoSet.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "OrbitClientModel/CaptureData.h"
#include "TimeGraph.h"

FrameTrackOnlineProcessor::FrameTrackOnlineProcessor(const CaptureData& capture_data,
                                                     TimeGraph* time_graph)
    : time_graph_(time_graph) {
  const auto& frame_track_function_ids = capture_data.frame_track_function_ids();
  for (const auto& function_id : frame_track_function_ids) {
    current_frame_track_function_ids_.insert(function_id);
    function_id_to_previous_timestamp_ns_.insert(
        std::make_pair(function_id, std::numeric_limits<uint64_t>::max()));
  }
}

void FrameTrackOnlineProcessor::ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                                             const orbit_client_protos::FunctionInfo& function) {
  uint64_t function_id = timer_info.function_id();
  if (!current_frame_track_function_ids_.contains(function_id)) {
    return;
  }
  uint64_t previous_timestamp_ns = function_id_to_previous_timestamp_ns_.at(function_id);
  if (previous_timestamp_ns == std::numeric_limits<uint64_t>::max()) {
    function_id_to_previous_timestamp_ns_[function_id] = timer_info.start();
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

    function_id_to_previous_timestamp_ns_[function_id] = timer_info.start();
  }
}

void FrameTrackOnlineProcessor::AddFrameTrack(uint64_t function_id) {
  current_frame_track_function_ids_.insert(function_id);
  function_id_to_previous_timestamp_ns_.insert(
      std::make_pair(function_id, std::numeric_limits<uint64_t>::max()));
}

void FrameTrackOnlineProcessor::RemoveFrameTrack(uint64_t function_id) {
  current_frame_track_function_ids_.erase(function_id);
  function_id_to_previous_timestamp_ns_.erase(function_id);
}
