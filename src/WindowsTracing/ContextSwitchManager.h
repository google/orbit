// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_CONTEXT_SWITCH_MANAGER_H_
#define WINDOWS_TRACING_CONTEXT_SWITCH_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_tracing {

// For each core, keeps the last context switch into a thread and matches it with the next context
// switch away from a thread to produce SchedulingSlice events. It assumes that context switches
// for the same core come in order and that thread events are received before cpu events.
class ContextSwitchManager {
 public:
  explicit ContextSwitchManager(TracerListener* listener) : listener_(listener) {}
  ContextSwitchManager() = delete;

  void ProcessTidToPidMapping(uint32_t tid, uint32_t pid);
  void ProcessContextSwitch(uint16_t cpu, uint32_t old_tid, uint32_t new_tid,
                            uint64_t timestamp_ns);
  void OutputStats();

  struct Stats {
    uint64_t num_processed_cpu_events_ = 0;
    uint64_t num_processed_thread_events_ = 0;
    uint64_t num_tid_mismatches_ = 0;
    uint64_t num_scheduling_slices = 0;
    uint64_t num_scheduling_slices_with_invalid_pid = 0;
    absl::flat_hash_set<uint32_t> tid_witout_pid_set_;
  };

  const Stats& GetStats() { return stats_; }

 private:
  struct ContextSwitch {
    uint64_t timestamp_ns = 0;
    uint32_t old_tid = 0;
    uint32_t new_tid = 0;
  };

  TracerListener* listener_ = nullptr;
  absl::flat_hash_map<uint32_t, uint32_t> pid_by_tid_;
  absl::flat_hash_map<uint32_t, ContextSwitch> last_context_switch_by_cpu_;
  Stats stats_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_CONTEXT_SWITCH_MANAGER_H_
