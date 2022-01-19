// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ContextSwitchManager.h"

#include <absl/meta/type_traits.h>
#include <stdint.h>

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::SchedulingSlice;

void ContextSwitchManager::ProcessContextSwitchIn(std::optional<pid_t> pid, pid_t tid,
                                                  uint16_t core, uint64_t timestamp_ns) {
  // In case of lost out switches, a previous OpenSwitchIn for this core can be already present.
  // Simply overwrite it.
  open_switches_by_core_.emplace(core, OpenSwitchIn{pid, tid, timestamp_ns});
}

std::optional<SchedulingSlice> ContextSwitchManager::ProcessContextSwitchOut(
    pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns) {
  auto open_switch_it = open_switches_by_core_.find(core);
  // This can happen at the beginning or in case of lost in switches.
  if (open_switch_it == open_switches_by_core_.end()) {
    return std::nullopt;
  }

  std::optional<pid_t> open_pid = open_switch_it->second.pid;
  pid_t open_tid = open_switch_it->second.tid;
  uint64_t open_timestamp_ns = open_switch_it->second.timestamp_ns;

  ORBIT_CHECK(timestamp_ns >= open_timestamp_ns);

  // Remove the OpenSwitchIn for this core before returning, as it will have been processed.
  open_switches_by_core_.erase(core);

  // This can happen in case of lost in/out switches.
  if ((open_pid.has_value() && pid != -1 && open_pid.value() != pid) || open_tid != tid) {
    return std::nullopt;
  }

  // When a context witch out is caused by a thread exiting, the perf_event_open event
  // has pid set to -1 (and also the tid, but we use the one from the tracepoint data):
  // in such case, use the pid from the OpenSwitchIn, if available.
  // If this is not available either, the pid will then just incorrectly be -1
  // (we prefer this to discarding the SchedulingSlice altogether).
  pid_t pid_to_set;
  if (pid != -1) {
    pid_to_set = pid;
  } else if (open_pid.has_value()) {
    pid_to_set = open_pid.value();
  } else {
    pid_to_set = -1;
  }

  SchedulingSlice scheduling_slice;
  scheduling_slice.set_pid(pid_to_set);
  scheduling_slice.set_tid(tid);
  scheduling_slice.set_core(core);
  scheduling_slice.set_duration_ns(timestamp_ns - open_timestamp_ns);
  scheduling_slice.set_out_timestamp_ns(timestamp_ns);
  return scheduling_slice;
}

}  // namespace orbit_linux_tracing
