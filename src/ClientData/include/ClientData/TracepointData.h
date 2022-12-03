// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TRACEPOINT_DATA_H_
#define CLIENT_DATA_TRACEPOINT_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "ClientData/TracepointEventInfo.h"
#include "ClientData/TracepointInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/tracepoint.pb.h"

namespace orbit_client_data {

/*
 * TracepointData stores all tracepoint related information on the Client/Ui side.
 * Single events in which a tracepoint got hit are represented by a 'TracepointEventInfo'.
 * These events are stored ordered by the time stamp of the event such that one can iterate over
 * the events in a time interval in an efficient manner.
 * The class offers methods to add events and iterate over over them.
 *
 * Other than the events themselves the class stores a description of each system trace point used
 * (`TracepointInfo`). This is stored in a map from integer to a `TracepointInfo`) and is used
 * to compress the wire format of events. The events contain an identifier rather than the full
 * description of the tracepoint they correspond to.
 *
 * Thread-Safety: This class is thread-safe.
 */
class TracepointData {
 public:
  // Assume that the corresponding tracepoint of tracepoint_id is already in unique_tracepoints_
  void EmplaceTracepointEvent(uint64_t timestamp_ns, uint64_t tracepoint_id, uint32_t process_id,
                              uint32_t thread_id, int32_t cpu, bool is_same_pid_as_target);

  void ForEachTracepointEventOfThreadInTimeRange(
      uint32_t thread_id, uint64_t min_tick, uint64_t max_tick_exclusive,
      const std::function<void(const TracepointEventInfo&)>& action) const;

  void ForEachTracepointEvent(const std::function<void(const TracepointEventInfo&)>& action) const;

  uint32_t GetNumTracepointEventsForThreadId(uint32_t thread_id) const;

  bool AddUniqueTracepointInfo(uint64_t key, TracepointInfo tracepoint);

  [[nodiscard]] const TracepointInfo* GetTracepointInfo(uint64_t tracepoint_id) const;
  [[nodiscard]] bool HasTracepointId(uint64_t tracepoint_id) const;

  void ForEachUniqueTracepointInfo(const std::function<void(const TracepointInfo&)>& action) const;

 private:
  int32_t num_total_tracepoint_events_ = 0;

  mutable absl::Mutex mutex_;
  mutable absl::Mutex unique_tracepoints_mutex_;

  absl::flat_hash_map<uint32_t, std::map<uint64_t, TracepointEventInfo>>
      thread_id_to_time_to_tracepoint_ ABSL_GUARDED_BY(mutex_);

  // Store unique pointers, such that we can hand out pointers to tracepoint infos, without
  // requiring the caller to lock the mutex.
  absl::flat_hash_map<uint64_t, std::unique_ptr<TracepointInfo>> unique_tracepoints_
      ABSL_GUARDED_BY(unique_tracepoints_mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TRACEPOINT_DATA_H_
