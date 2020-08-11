// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ContextSwitchManager.h"

namespace LinuxTracing {

using orbit_grpc_protos::SchedulingSlice;

void ContextSwitchManager::ProcessContextSwitchIn(pid_t pid, pid_t tid,
                                                  uint16_t core,
                                                  uint64_t timestamp_ns) {
  // In case of lost out switches, a previous OpenSwitchIn for this core can
  // be present. Simply overwrite it.
  open_switches_by_core_.emplace(core, OpenSwitchIn{pid, tid, timestamp_ns});
}

std::optional<SchedulingSlice> ContextSwitchManager::ProcessContextSwitchOut(
    pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns) {
  auto open_switch_it = open_switches_by_core_.find(core);
  // This can happen at the beginning or in case of lost in switches.
  if (open_switch_it == open_switches_by_core_.end()) {
    return std::nullopt;
  }

  pid_t open_pid = open_switch_it->second.pid;
  pid_t open_tid = open_switch_it->second.tid;
  uint64_t open_timestamp_ns = open_switch_it->second.timestamp_ns;

  CHECK(timestamp_ns >= open_timestamp_ns);

  // Remove the OpenSwitchIn for this core before returning,
  // as it will have been processed.
  open_switches_by_core_.erase(core);

  // When a context switch out is caused by a thread exiting, the
  // perf_event_open event has pid and tid set to -1:
  // in such case, use pid and tid from the OpenSwitchIn.
  if (pid == -1 || tid == -1) {
    SchedulingSlice scheduling_slice;
    scheduling_slice.set_pid(open_pid);
    scheduling_slice.set_tid(open_tid);
    scheduling_slice.set_core(core);
    scheduling_slice.set_in_timestamp_ns(open_timestamp_ns);
    scheduling_slice.set_out_timestamp_ns(timestamp_ns);
    return scheduling_slice;
  }

  // This can happen in case of lost in/out switches.
  if (open_pid != pid || open_tid != tid) {
    return std::nullopt;
  }

  SchedulingSlice scheduling_slice;
  scheduling_slice.set_pid(pid);
  scheduling_slice.set_tid(tid);
  scheduling_slice.set_core(core);
  scheduling_slice.set_in_timestamp_ns(open_timestamp_ns);
  scheduling_slice.set_out_timestamp_ns(timestamp_ns);
  return scheduling_slice;
}

}  // namespace LinuxTracing
