// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_
#define ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_

#include <cstdint>

#include "OrbitClientModel/CaptureData.h"
#include "TimeGraph.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

class CaptureData;
class TimeGraph;

// FrameTrackOnlineProcessor is used to create frame track timers during a capture.
class FrameTrackOnlineProcessor {
 public:
  FrameTrackOnlineProcessor() : time_graph_(nullptr) {}
  FrameTrackOnlineProcessor(const CaptureData& capture_data, TimeGraph* time_graph);
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                    const orbit_client_protos::FunctionInfo& function);

 private:
  absl::flat_hash_set<uint64_t> current_frame_track_functions_;
  absl::flat_hash_map<uint64_t, uint64_t> previous_timestamp_ns_;

  TimeGraph* time_graph_;
  int current_frame_index_ = 0;
};

#endif  // ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_