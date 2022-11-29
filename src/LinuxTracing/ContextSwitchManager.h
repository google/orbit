// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_
#define LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <stdint.h>
#include <sys/types.h>

#include <cstdint>
#include <optional>

#include "GrpcProtos/capture.pb.h"

namespace orbit_linux_tracing {

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

  void ProcessContextSwitchIn(std::optional<pid_t> pid, pid_t tid, uint16_t core,
                              uint64_t timestamp_ns);

  std::optional<orbit_grpc_protos::SchedulingSlice> ProcessContextSwitchOut(pid_t pid, pid_t tid,
                                                                            uint16_t core,
                                                                            uint64_t timestamp_ns);

 private:
  struct OpenSwitchIn {
    OpenSwitchIn(std::optional<pid_t> pid, pid_t tid, uint64_t timestamp_ns)
        : pid{pid}, tid{tid}, timestamp_ns{timestamp_ns} {}
    std::optional<pid_t> pid;
    pid_t tid;
    uint64_t timestamp_ns;
  };

  absl::flat_hash_map<uint16_t, OpenSwitchIn> open_switches_by_core_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_CONTEXT_SWITCH_MANAGER_H_
