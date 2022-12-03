// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_THREAD_STATE_MANAGER_H_
#define LINUX_TRACING_THREAD_STATE_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <stdint.h>
#include <sys/types.h>

#include <optional>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "absl/container/flat_hash_map.h"

namespace orbit_linux_tracing {

// ThreadStateManager stores the state of threads, handles the state transitions,
// builds and returns ThreadStateSlices.
// The following diagram shows the relationship between the states and the tracepoints.
// Note that, for some state transitions, multiple tracepoints could be used
// (e.g., both sched:sched_waking and sched:sched_wakeup for "not runnable" to "runnable").
// The diagram indicates them all but we only use the ones not in parentheses.
// Also note we don't have a transition out of the diagram for a thread that exits.
// Instead, a thread that has exited will remain "not runnable" with state "dead"
// or sometimes "zombie". This is mostly for simplicity reasons,
// as a thread that exits first goes through sched:sched_process_exit,
// but then still goes through one or often multiple sched:sched_switches.
//
//       task:task_newtask                             sched:sched_switch(in)
//   (OR sched:sched_wakeup_new)    ------------ -------------------------------> -----------
// -------------------------------> | Runnable |                                  | Running |
//                                  ------------ <------------------------------- -----------
//                                       ^            sched:sched_switch(out)       ^  |
//                                       |             with prev_state=='R'         .  |
//                                       |                                          .  |
//                                       |                   sched:sched_switch(in) .  |
//                                       |               ---------------- . . . . . .  |
//                                       |               | Not runnable |              |
//                                       --------------- | incl. exited | <-------------
//                                sched:sched_wakeup     ----------------    sched_switch(out)
//                             (OR sched:sched_waking)                      with prev_state!='R'
//                                                                   (ALSO sched:sched_process_exit)

class ThreadStateManager {
 public:
  void OnInitialState(uint64_t timestamp_ns, pid_t tid,
                      orbit_grpc_protos::ThreadStateSlice::ThreadState state);
  void OnNewTask(uint64_t timestamp_ns, pid_t tid, pid_t was_created_by_tid,
                 pid_t was_created_by_pid);
  [[nodiscard]] std::optional<orbit_grpc_protos::ThreadStateSlice> OnSchedWakeup(
      uint64_t timestamp_ns, pid_t tid, pid_t was_unblocked_by_tid, pid_t was_unblocked_by_pid,
      bool has_wakeup_callstack = false);
  [[nodiscard]] std::optional<orbit_grpc_protos::ThreadStateSlice> OnSchedSwitchIn(
      uint64_t timestamp_ns, pid_t tid);
  [[nodiscard]] std::optional<orbit_grpc_protos::ThreadStateSlice> OnSchedSwitchOut(
      uint64_t timestamp_ns, pid_t tid, orbit_grpc_protos::ThreadStateSlice::ThreadState new_state,
      bool has_switch_out_callstack = false);
  [[nodiscard]] std::vector<orbit_grpc_protos::ThreadStateSlice> OnCaptureFinished(
      uint64_t timestamp_ns);

 private:
  struct OpenState {
    OpenState(orbit_grpc_protos::ThreadStateSlice::ThreadState state, uint64_t begin_timestamp_ns,
              bool has_wakeup_or_switch_out_callstack = false)
        : state{state},
          begin_timestamp_ns{begin_timestamp_ns},
          wakeup_reason{orbit_grpc_protos::ThreadStateSlice::kNotApplicable},
          wakeup_tid{0},
          wakeup_pid{0},
          has_wakeup_or_switch_out_callstack{has_wakeup_or_switch_out_callstack} {}
    OpenState(orbit_grpc_protos::ThreadStateSlice::ThreadState state, uint64_t begin_timestamp_ns,
              orbit_grpc_protos::ThreadStateSlice::WakeupReason wakeup_reason, pid_t wakeup_tid,
              pid_t wakeup_pid, bool has_wakeup_or_switch_out_callstack = false)
        : state{state},
          begin_timestamp_ns{begin_timestamp_ns},
          wakeup_reason{wakeup_reason},
          wakeup_tid{wakeup_tid},
          wakeup_pid{wakeup_pid},
          has_wakeup_or_switch_out_callstack{has_wakeup_or_switch_out_callstack} {}
    orbit_grpc_protos::ThreadStateSlice::ThreadState state;
    uint64_t begin_timestamp_ns;
    // The following field explains the relation between this thread and the thread that woke it up
    // (identified by wakeup_tid and wakeup_pid below).
    orbit_grpc_protos::ThreadStateSlice::WakeupReason wakeup_reason;
    // The next two fields are optional, and only meaningful when wakeup_reason != kNotApplicable.
    // We use them to indicate which tid and pid caused the thread to transition from a non-runnable
    // to the runnable state.
    pid_t wakeup_tid;
    pid_t wakeup_pid;
    // We allow the user to collect callstacks on sched_wakeup and sched_switch out events. This
    // field indicates if there was a callstack collected together with this open state. The
    // callstack itself gets processed in the UprobesUnwindingVisitor, but this field indicates
    // if we will need to wait for this callstack.
    bool has_wakeup_or_switch_out_callstack;
  };

  absl::flat_hash_map<pid_t, OpenState> tid_open_states_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_THREAD_STATE_MANAGER_H_
