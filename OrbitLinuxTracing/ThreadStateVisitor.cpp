// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadStateVisitor.h"

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitLinuxTracing/TracerListener.h"

namespace LinuxTracing {

using orbit_grpc_protos::ThreadStateSlice;

void ThreadStateVisitor::ProcessInitialTidToPidAssociation(pid_t tid, pid_t pid) {
  bool new_insertion = tid_to_pid_association_.insert_or_assign(tid, pid).second;
  if (!new_insertion) {
    ERROR("Overwriting previous pid for tid %d with initial pid %d", tid, pid);
  }
}

void ThreadStateVisitor::visit(ForkPerfEvent* event) {
  pid_t pid = event->GetPid();
  pid_t tid = event->GetTid();
  bool new_insertion = tid_to_pid_association_.insert_or_assign(tid, pid).second;
  if (!new_insertion) {
    ERROR("Overwriting previous pid for tid %d with pid %d from PERF_RECORD_FORK", tid, pid);
  }
}

bool ThreadStateVisitor::TidMatchesPidFilter(pid_t tid) {
  if (pid_filter_ == kPidFilterNoThreadState) {
    return false;
  }

  auto tid_to_pid_it = tid_to_pid_association_.find(tid);
  if (tid_to_pid_it == tid_to_pid_association_.end()) {
    return false;
  }

  return tid_to_pid_it->second == pid_filter_;
}

void ThreadStateVisitor::ProcessInitialState(uint64_t timestamp_ns, pid_t tid, char state_char) {
  if (!TidMatchesPidFilter(tid)) {
    return;
  }

  std::optional<ThreadStateSlice::ThreadState> initial_state = GetThreadStateFromChar(state_char);
  if (!initial_state.has_value()) {
    ERROR("Parsing thread state char '%c' for tid %d", state_char, tid);
    return;
  }
  state_manager_.OnInitialState(timestamp_ns, tid, initial_state.value());
}

void ThreadStateVisitor::visit(TaskNewtaskPerfEvent* event) {
  if (!TidMatchesPidFilter(event->GetTid())) {
    return;
  }
  state_manager_.OnNewTask(event->GetTimestamp(), event->GetTid());
}

void ThreadStateVisitor::visit(SchedSwitchPerfEvent* event) {
  // Switches with tid 0 are associated with idle CPU, don't consider them.
  if (event->GetPrevTid() != 0 && TidMatchesPidFilter(event->GetPrevTid())) {
    ThreadStateSlice::ThreadState new_state = GetThreadStateFromBits(event->GetPrevState());
    std::optional<ThreadStateSlice> out_slice =
        state_manager_.OnSchedSwitchOut(event->GetTimestamp(), event->GetPrevTid(), new_state);
    if (out_slice.has_value()) {
      CHECK(listener_ != nullptr);
      listener_->OnThreadStateSlice(std::move(out_slice.value()));
    }
  }

  if (event->GetNextTid() != 0 && TidMatchesPidFilter(event->GetNextTid())) {
    std::optional<ThreadStateSlice> in_slice =
        state_manager_.OnSchedSwitchIn(event->GetTimestamp(), event->GetNextTid());
    if (in_slice.has_value()) {
      CHECK(listener_ != nullptr);
      listener_->OnThreadStateSlice(std::move(in_slice.value()));
    }
  }
}

void ThreadStateVisitor::visit(SchedWakeupPerfEvent* event) {
  if (!TidMatchesPidFilter(event->GetWokenTid())) {
    return;
  }

  std::optional<ThreadStateSlice> state_slice =
      state_manager_.OnSchedWakeup(event->GetTimestamp(), event->GetWokenTid());
  if (state_slice.has_value()) {
    CHECK(listener_ != nullptr);
    listener_->OnThreadStateSlice(std::move(state_slice.value()));
  }
}

void ThreadStateVisitor::ProcessRemainingOpenStates(uint64_t timestamp_ns) {
  std::vector<ThreadStateSlice> state_slices = state_manager_.OnCaptureFinished(timestamp_ns);
  for (ThreadStateSlice& slice : state_slices) {
    CHECK(listener_ != nullptr);
    listener_->OnThreadStateSlice(slice);
  }
}

// Associates a ThreadStateSlice::ThreadState to a thread state character retrieved from
// /proc/<pid>/stat or the `ps` command. The possible characters are obtained from
// /sys/kernel/debug/tracing/events/sched/sched_switch/format and compared with the ones listed in
// https://man7.org/linux/man-pages/man5/proc.5.html and
// https://www.man7.org/linux/man-pages/man1/ps.1.html#PROCESS_STATE_CODES to make sure we are not
// missing any additional valid one.
std::optional<ThreadStateSlice::ThreadState> ThreadStateVisitor::GetThreadStateFromChar(char c) {
  switch (c) {
    case 'R':
      return ThreadStateSlice::kRunnable;
    case 'S':
      return ThreadStateSlice::kInterruptibleSleep;
    case 'D':
      return ThreadStateSlice::kUninterruptibleSleep;
    case 'T':
      return ThreadStateSlice::kStopped;
    case 't':
      return ThreadStateSlice::kTraced;
    case 'X':
      return ThreadStateSlice::kDead;
    case 'Z':
      return ThreadStateSlice::kZombie;
    case 'P':
      // Note that 'P' (Parked) is only valid from Linux 3.9 to 3.13, but we still include it as it
      // is mentioned in /sys/kernel/debug/tracing/events/sched/sched_switch/format and in
      // https://github.com/torvalds/linux/blob/master/fs/proc/array.c.
      return ThreadStateSlice::kParked;
    case 'I':
      // 'I' (Idle) only applies to kernel threads. See
      // https://github.com/torvalds/linux/commit/06eb61844d841d0032a9950ce7f8e783ee49c0d0.
      return ThreadStateSlice::kIdle;
    default:
      return std::nullopt;
  }
}

// Associates a ThreadStateSlice::ThreadState to the bits of the prev_state field of the
// sched:sched_switch tracepoint. The association is given away by "print fmt" in
// /sys/kernel/debug/tracing/events/sched/sched_switch/format or by
// https://github.com/torvalds/linux/blob/master/fs/proc/array.c.
ThreadStateSlice::ThreadState ThreadStateVisitor::GetThreadStateFromBits(uint64_t bits) {
  if (__builtin_popcountl(bits & 0xFF) > 1) {
    ERROR("The thread state mask %#lx is a combination of states, reporting only the first",
          bits & 0xFF);
  }
  if ((bits & 0x01) != 0) return ThreadStateSlice::kInterruptibleSleep;
  if ((bits & 0x02) != 0) return ThreadStateSlice::kUninterruptibleSleep;
  if ((bits & 0x04) != 0) return ThreadStateSlice::kStopped;
  if ((bits & 0x08) != 0) return ThreadStateSlice::kTraced;
  if ((bits & 0x10) != 0) return ThreadStateSlice::kDead;
  if ((bits & 0x20) != 0) return ThreadStateSlice::kZombie;
  if ((bits & 0x40) != 0) return ThreadStateSlice::kParked;
  if ((bits & 0x80) != 0) return ThreadStateSlice::kIdle;
  return ThreadStateSlice::kRunnable;
}

}  // namespace LinuxTracing
