// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_
#define ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>

#include <cstdint>

#include "ClientData/CaptureData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitGl/TimeGraph.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

namespace orbit_gl {

void CreateFrameTrackTimer(uint64_t function_id, uint64_t start_ns, uint64_t end_ns, int frame_id,
                           orbit_client_protos::TimerInfo* timer_info);

// FrameTrackOnlineProcessor is used to create frame track timers during a capture.
class FrameTrackOnlineProcessor {
 public:
  FrameTrackOnlineProcessor() = default;
  FrameTrackOnlineProcessor(const orbit_client_data::CaptureData& capture_data,
                            TimeGraph* time_graph);
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info);

  void AddFrameTrack(uint64_t function_id);
  void RemoveFrameTrack(uint64_t function_id);

 private:
  absl::flat_hash_set<uint64_t> current_frame_track_function_ids_;
  absl::flat_hash_map<uint64_t, uint64_t> function_id_to_previous_timestamp_ns_;

  TimeGraph* time_graph_{nullptr};
  int current_frame_index_ = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_FRAME_TRACK_ONLINE_PROCESSOR_H_
