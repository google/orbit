// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_
#define ORBIT_LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_

#include <OrbitBase/Logging.h>

#include "absl/container/flat_hash_map.h"
#include "capture.pb.h"

namespace LinuxTracing {

// For each core, keeps the last context switch into a process and matches it
// with the next context switch away from a process to produce SchedulingSlice
// events. It assumes that context switches for the same core come in order.
class ContextSwitchManager {
 public:
  ContextSwitchManager() = default;

  ContextSwitchManager(const ContextSwitchManager&) = delete;
  ContextSwitchManager& operator=(const ContextSwitchManager&) = delete;

  ContextSwitchManager(ContextSwitchManager&&) = default;
  ContextSwitchManager& operator=(ContextSwitchManager&&) = default;

  void ProcessContextSwitchIn(pid_t pid, pid_t tid, uint16_t core,
                              uint64_t timestamp_ns);

  std::optional<orbit_grpc_protos::SchedulingSlice> ProcessContextSwitchOut(
      pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns);

  void Clear() { open_switches_by_core_.clear(); }

 private:
  struct OpenSwitchIn {
    OpenSwitchIn(pid_t pid, pid_t tid, uint64_t timestamp_ns)
        : pid(pid), tid(tid), timestamp_ns(timestamp_ns) {}
    pid_t pid;
    pid_t tid;
    uint64_t timestamp_ns;
  };

  absl::flat_hash_map<uint16_t, OpenSwitchIn> open_switches_by_core_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_
