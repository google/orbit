// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadStateVisitor.h"

namespace LinuxTracing {

using orbit_grpc_protos::ThreadStateSlice;

// Note: Since we use PerfEventProcessor to process perf_event_open events in order, OnNewTask,
// OnSchedWakeup, OnSchedSwitchIn, OnSchedSwitchOut are expected to be called in order of
// timestamp. But the initial thread states are retrieved (and OnInitialState is called) after the
// perf_event_open file descriptors have been enabled, so that we don't lose thread states between
// retrieving the initial states and enabling the file descriptors. It is then common for some of
// the first tracepoint events to have a timestamp lower than the timestamp of initial retrieval. In
// all these cases, we discard the previous known state (the one retrieved at the beginning, with a
// larger timestamp) and replace it with the thread state carried by the tracepoint.

void ThreadStateManager::OnInitialState(uint64_t timestamp_ns, pid_t tid,
                                        ThreadStateSlice::ThreadState state) {
  CHECK(!tid_open_states_.contains(tid));
  tid_open_states_.emplace(tid, OpenState{state, timestamp_ns});
}

void ThreadStateManager::OnNewTask(uint64_t timestamp_ns, pid_t tid) {
  static constexpr ThreadStateSlice::ThreadState kNewState = ThreadStateSlice::kRunnable;

  if (auto open_state_it = tid_open_states_.find(tid);
      open_state_it != tid_open_states_.end() &&
      timestamp_ns >= open_state_it->second.begin_timestamp_ns) {
    ERROR("Processed task:task_newtask but thread %d was already known", tid);
    return;
  }
  tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
}

std::optional<ThreadStateSlice> ThreadStateManager::OnSchedWakeup(uint64_t timestamp_ns,
                                                                  pid_t tid) {
  static constexpr ThreadStateSlice::ThreadState kNewState = ThreadStateSlice::kRunnable;

  auto open_state_it = tid_open_states_.find(tid);
  if (open_state_it == tid_open_states_.end()) {
    ERROR("Processed sched:sched_wakeup but previous state of thread %d is unknown", tid);
    tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
    return std::nullopt;
  }

  const OpenState& open_state = open_state_it->second;
  if (timestamp_ns < open_state.begin_timestamp_ns) {
    // As noted above, overwrite the thread state retrieved at the beginning.
    tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
    return std::nullopt;
  }

  if (open_state.state == kNewState || open_state.state == ThreadStateSlice::kRunning) {
    // It seems to be somewhat common for a thread to receive a wakeup
    // while already in runnable or running state: disregard the state change.
    return std::nullopt;
  }

  if (open_state.state == ThreadStateSlice::kZombie ||
      open_state.state == ThreadStateSlice::kDead) {
    ERROR("Processed sched:sched_wakeup for thread %d but unexpected previous state %s", tid,
          ThreadStateSlice::ThreadState_Name(open_state.state));
  }

  ThreadStateSlice slice;
  slice.set_tid(tid);
  slice.set_thread_state(open_state.state);
  slice.set_begin_timestamp_ns(open_state.begin_timestamp_ns);
  slice.set_end_timestamp_ns(timestamp_ns);
  tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
  return slice;
}

std::optional<ThreadStateSlice> ThreadStateManager::OnSchedSwitchIn(uint64_t timestamp_ns,
                                                                    pid_t tid) {
  static constexpr ThreadStateSlice::ThreadState kNewState = ThreadStateSlice::kRunning;

  auto open_state_it = tid_open_states_.find(tid);
  if (open_state_it == tid_open_states_.end()) {
    ERROR("Processed sched:sched_switch(in) but previous state of thread %d is unknown", tid);
    tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
    return std::nullopt;
  }

  const OpenState& open_state = open_state_it->second;
  if (timestamp_ns < open_state.begin_timestamp_ns) {
    tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
    return std::nullopt;
  }

  if (open_state.state == kNewState) {
    // No state change: do nothing and don't overwrite the previous begin timestamp.
    return std::nullopt;
  }

  // Don't print an error even if open_state.state != ThreadStateSlice::kRunnable: it seems to be
  // sometimes possible for a thread to go from a non-runnable state directly to running, skipping
  // the sched:sched_wakeup event.

  ThreadStateSlice slice;
  slice.set_tid(tid);
  slice.set_thread_state(open_state.state);
  slice.set_begin_timestamp_ns(open_state.begin_timestamp_ns);
  slice.set_end_timestamp_ns(timestamp_ns);
  tid_open_states_.insert_or_assign(tid, OpenState{kNewState, timestamp_ns});
  return slice;
}

std::optional<ThreadStateSlice> ThreadStateManager::OnSchedSwitchOut(
    uint64_t timestamp_ns, pid_t tid, ThreadStateSlice::ThreadState new_state) {
  auto open_state_it = tid_open_states_.find(tid);
  if (open_state_it == tid_open_states_.end()) {
    ERROR("Processed sched:sched_switch(out) but previous state of thread %d is unknown", tid);
    tid_open_states_.insert_or_assign(tid, OpenState{new_state, timestamp_ns});
    return std::nullopt;
  }

  const OpenState& open_state = open_state_it->second;
  if (timestamp_ns < open_state.begin_timestamp_ns) {
    tid_open_states_.insert_or_assign(tid, OpenState{new_state, timestamp_ns});
    return std::nullopt;
  }

  // As we are switching out of a CPU, if the previous state was kRunnable, assume it was kRunning.
  // This is because when we retrieve the initial thread states we have no way to distinguish
  // between kRunnable and kRunning. After all, for the OS they are the same state.
  ThreadStateSlice::ThreadState adjusted_open_state_state = open_state.state;
  if (adjusted_open_state_state == ThreadStateSlice::kRunnable) {
    adjusted_open_state_state = ThreadStateSlice::kRunning;
  }

  if (adjusted_open_state_state != ThreadStateSlice::kRunning) {
    ERROR("Processed sched:sched_switch(out) for thread %d but unexpected previous state %s", tid,
          ThreadStateSlice::ThreadState_Name(adjusted_open_state_state));
    if (adjusted_open_state_state == new_state) {
      // No state change: do nothing and don't overwrite the previous begin timestamp.
      return std::nullopt;
    }
  }

  ThreadStateSlice slice;
  slice.set_tid(tid);
  slice.set_thread_state(adjusted_open_state_state);
  slice.set_begin_timestamp_ns(open_state.begin_timestamp_ns);
  slice.set_end_timestamp_ns(timestamp_ns);

  // Note: If the thread exits but the new_state is kZombie instead of kDead,
  // the switch to kDead will never be reported.
  tid_open_states_.insert_or_assign(tid, OpenState{new_state, timestamp_ns});
  return slice;
}

std::vector<ThreadStateSlice> ThreadStateManager::OnCaptureFinished(uint64_t timestamp_ns) {
  std::vector<ThreadStateSlice> slices;
  for (const auto& [tid, open_state] : tid_open_states_) {
    ThreadStateSlice slice;
    slice.set_tid(tid);
    slice.set_thread_state(open_state.state);
    slice.set_begin_timestamp_ns(open_state.begin_timestamp_ns);
    slice.set_end_timestamp_ns(timestamp_ns);
    slices.emplace_back(std::move(slice));
  }
  return slices;
}

void ThreadStateVisitor::ProcessInitialState(uint64_t timestamp_ns, pid_t tid, char state_char) {
  std::optional<ThreadStateSlice::ThreadState> initial_state = GetThreadStateFromChar(state_char);
  if (!initial_state.has_value()) {
    ERROR("Parsing thread state char '%c' for tid %d", state_char, tid);
    return;
  }
  state_manager_.OnInitialState(timestamp_ns, tid, initial_state.value());
}

void ThreadStateVisitor::visit(TaskNewtaskPerfEvent* event) {
  CHECK(listener_ != nullptr);
  state_manager_.OnNewTask(event->GetTimestamp(), event->GetTid());
}

void ThreadStateVisitor::visit(SchedSwitchPerfEvent* event) {
  CHECK(listener_ != nullptr);
  // Switches with tid 0 are associated with idle CPU, don't consider them.
  if (event->GetPrevTid() != 0) {
    ThreadStateSlice::ThreadState new_state = GetThreadStateFromBits(event->GetPrevState());
    std::optional<ThreadStateSlice> out_slice =
        state_manager_.OnSchedSwitchOut(event->GetTimestamp(), event->GetPrevTid(), new_state);
    if (out_slice.has_value()) {
      listener_->OnThreadStateSlice(std::move(out_slice.value()));
    }
  }
  if (event->GetNextTid() != 0) {
    std::optional<ThreadStateSlice> in_slice =
        state_manager_.OnSchedSwitchIn(event->GetTimestamp(), event->GetNextTid());
    if (in_slice.has_value()) {
      listener_->OnThreadStateSlice(std::move(in_slice.value()));
    }
  }
}

void ThreadStateVisitor::visit(SchedWakeupPerfEvent* event) {
  CHECK(listener_ != nullptr);
  std::optional<ThreadStateSlice> state_slice =
      state_manager_.OnSchedWakeup(event->GetTimestamp(), event->GetWokenTid());
  if (state_slice.has_value()) {
    listener_->OnThreadStateSlice(std::move(state_slice.value()));
  }
}

void ThreadStateVisitor::ProcessRemainingOpenStates(uint64_t timestamp_ns) {
  CHECK(listener_ != nullptr);
  std::vector<ThreadStateSlice> state_slices = state_manager_.OnCaptureFinished(timestamp_ns);
  for (ThreadStateSlice& slice : state_slices) {
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
