// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/FrameTrackOnlineProcessor.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <limits>
#include <utility>

#include "ClientData/CaptureData.h"
#include "OrbitGl/TimeGraph.h"

namespace orbit_gl {

void CreateFrameTrackTimer(uint64_t function_id, uint64_t start_ns, uint64_t end_ns, int frame_id,
                           orbit_client_protos::TimerInfo* timer_info) {
  // TID is meaningless for this timer (start and end can be on two different threads).
  constexpr const int32_t kUnusedThreadId = -1;
  timer_info->set_thread_id(kUnusedThreadId);
  timer_info->set_function_id(function_id);
  timer_info->set_start(start_ns);
  timer_info->set_end(end_ns);
  // We use user_data_key to keep track of the frame number.
  timer_info->set_user_data_key(frame_id);
  timer_info->set_type(orbit_client_protos::TimerInfo::kFrame);
}

FrameTrackOnlineProcessor::FrameTrackOnlineProcessor(
    const orbit_client_data::CaptureData& capture_data, TimeGraph* time_graph)
    : time_graph_(time_graph) {
  const auto& frame_track_function_ids = capture_data.frame_track_function_ids();
  for (const auto& function_id : frame_track_function_ids) {
    current_frame_track_function_ids_.insert(function_id);
    function_id_to_previous_timestamp_ns_.insert(
        std::make_pair(function_id, std::numeric_limits<uint64_t>::max()));
  }
}

void FrameTrackOnlineProcessor::ProcessTimer(const orbit_client_protos::TimerInfo& timer_info) {
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
    CreateFrameTrackTimer(function_id, previous_timestamp_ns, timer_info.start(),
                          current_frame_index_++, &frame_timer);
    time_graph_->ProcessTimer(frame_timer);
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

}  // namespace orbit_gl