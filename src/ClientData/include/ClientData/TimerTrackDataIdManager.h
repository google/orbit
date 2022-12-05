// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_TRACK_DATA_ID_MANAGER_H_
#define CLIENT_DATA_TIMER_TRACK_DATA_ID_MANAGER_H_

#include <OrbitBase/Logging.h>
#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>

#include "ClientProtos/capture_data.pb.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

// Manages track_id for different types of track;
class TimerTrackDataIdManager {
 public:
  TimerTrackDataIdManager();
  [[nodiscard]] uint32_t GenerateTrackIdFromTimerInfo(const TimerInfo& timer_info);

  [[nodiscard]] uint32_t GenerateSchedulerTrackId() const { return scheduler_track_id_; }
  [[nodiscard]] uint32_t GenerateFrameTrackId(uint64_t function_id);
  [[nodiscard]] uint32_t GenerateGpuTrackId(uint64_t timeline_hash);
  [[nodiscard]] uint32_t GenerateAsyncTrackId(std::string_view name);
  [[nodiscard]] uint32_t GenerateThreadTrackId(uint32_t thread_id);

 private:
  mutable absl::Mutex mutex_;
  uint32_t next_track_id_ = 0;

  uint32_t scheduler_track_id_;
  absl::flat_hash_map<uint64_t, uint32_t> frame_track_ids_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<uint64_t, uint32_t> gpu_track_ids_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<std::string, uint32_t> async_track_ids_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<uint32_t, uint32_t> thread_track_ids_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_TRACK_DATA_ID_MANAGER_H_
