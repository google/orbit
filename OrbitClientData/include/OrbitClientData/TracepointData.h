// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
#define ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <vector>

#include "capture_data.pb.h"
#include "tracepoint.pb.h"

/*
 * Stores `TracepointEventInfos` and a mapping from tracepoint key to unique `TracepointInfo`s.
 * It provides methods to add, access and iterate over those.
 *
 * Thread-Safety: This class is thread-safe.
 */
class TracepointData {
 public:
  // Assume that the corrspnding tracepoint of tracepoint_hash is already in unique_tracepoints_
  void EmplaceTracepointEvent(uint64_t time, uint64_t tracepoint_hash, int32_t process_id,
                              int32_t thread_id, int32_t cpu, bool is_same_pid_as_target);

  void ForEachTracepointEventOfThreadInTimeRange(
      int32_t thread_id, uint64_t min_tick, uint64_t max_tick_exclusive,
      const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const;

  void ForEachTracepointEvent(
      const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const;

  uint32_t GetNumTracepointsForThreadId(int32_t thread_id) const;

  bool AddUniqueTracepointInfo(uint64_t key, orbit_grpc_protos::TracepointInfo tracepoint);

  [[nodiscard]] orbit_grpc_protos::TracepointInfo GetTracepointInfo(uint64_t hash) const;
  [[nodiscard]] bool HasTracepointKey(uint64_t key) const;

  void ForEachUniqueTracepointInfo(
      const std::function<void(const orbit_client_protos::TracepointInfo&)>& action) const;

 private:
  int32_t num_total_tracepoints_ = 0;

  mutable absl::Mutex mutex_;
  mutable absl::Mutex unique_tracepoints_mutex_;

  absl::flat_hash_map<int32_t, std::map<uint64_t, orbit_client_protos::TracepointEventInfo> >
      thread_id_to_time_to_tracepoint_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::TracepointInfo> unique_tracepoints_;
};

#endif  // ORBIT_CORE_TRACEPOINT_EVENT_BUFFER_H_
