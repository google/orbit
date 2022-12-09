// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimerTrackDataIdManager.h"

#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

TimerTrackDataIdManager::TimerTrackDataIdManager() : scheduler_track_id_(next_track_id_++) {
  // Since there always will be an only scheduler track, we can assign it now.
}

uint32_t TimerTrackDataIdManager::GenerateTrackIdFromTimerInfo(const TimerInfo& timer_info) {
  switch (timer_info.type()) {
    case TimerInfo::kNone:
    case TimerInfo::kApiScope:
    case TimerInfo::kApiEvent:
      return GenerateThreadTrackId(timer_info.thread_id());
    case TimerInfo::kCoreActivity:
      return GenerateSchedulerTrackId();
    case TimerInfo::kFrame:
      return GenerateFrameTrackId(timer_info.function_id());
    case TimerInfo::kGpuActivity:
    case TimerInfo::kGpuCommandBuffer:
    case TimerInfo::kGpuDebugMarker:
      return GenerateGpuTrackId(timer_info.timeline_hash());
    case TimerInfo::kApiScopeAsync:
      return GenerateAsyncTrackId(timer_info.api_scope_name());
    case orbit_client_protos::TimerInfo_Type_TimerInfo_Type_INT_MIN_SENTINEL_DO_NOT_USE_:
    case orbit_client_protos::TimerInfo_Type_TimerInfo_Type_INT_MAX_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
  }
  return -1;
}

uint32_t TimerTrackDataIdManager::GenerateFrameTrackId(uint64_t function_id) {
  absl::MutexLock lock(&mutex_);
  auto [it, inserted] = frame_track_ids_.try_emplace(function_id, next_track_id_);
  if (inserted) {
    next_track_id_++;
  }
  return it->second;
}

uint32_t TimerTrackDataIdManager::GenerateGpuTrackId(uint64_t timeline_hash) {
  absl::MutexLock lock(&mutex_);
  auto [it, inserted] = gpu_track_ids_.try_emplace(timeline_hash, next_track_id_);
  if (inserted) {
    next_track_id_++;
  }
  return it->second;
}

uint32_t TimerTrackDataIdManager::GenerateAsyncTrackId(std::string_view name) {
  absl::MutexLock lock(&mutex_);
  auto [it, inserted] = async_track_ids_.try_emplace(name, next_track_id_);
  if (inserted) {
    next_track_id_++;
  }
  return it->second;
}

uint32_t TimerTrackDataIdManager::GenerateThreadTrackId(uint32_t thread_id) {
  absl::MutexLock lock(&mutex_);
  auto [it, inserted] = thread_track_ids_.try_emplace(thread_id, next_track_id_);
  if (inserted) {
    next_track_id_++;
  }
  return it->second;
}

}  // namespace orbit_client_data
