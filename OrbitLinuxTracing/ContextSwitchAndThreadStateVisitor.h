// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
#define ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "ContextSwitchManager.h"
#include "OrbitLinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "ThreadStateManager.h"
#include "capture.pb.h"

namespace LinuxTracing {

// This PerfEventVisitor visits PerfEvents associated with scheduling slices and thread states,
// processes them using ContextSwitchManager and ThreadStateManager, and passes the results to the
// specified TracerListener.
// As for some of these events the process id is not available, but only the thread id, this class
// also keeps the association between tids and pids system wide. The initial association extracted
// from the proc filesystem is passed by calling ProcessInitialTidToPidAssociation for each thread,
// and is updated with ForkPerfEvents (and also ExitPerfEvents, see visit(ExitPerfEvent*)).
// For thread states, we are able to collect partial slices at the beginning and at the end of the
// capture, hence the ProcessInitialState and ProcessRemainingOpenStates methods.
// Also, we only process thread states of the process with pid specified with
// SetThreadStatePidFilter (so that we can collect thread states only for the process we are
// profiling). For this we also need the system-wide association between tids and pids.
class ContextSwitchAndThreadStateVisitor : public PerfEventVisitor {
 public:
  void SetListener(TracerListener* listener) { listener_ = listener; }
  void SetThreadStatePidFilter(pid_t pid) { thread_state_pid_filter_ = pid; }

  void ProcessInitialTidToPidAssociation(pid_t tid, pid_t pid);
  void visit(ForkPerfEvent* event) override;
  void visit(ExitPerfEvent* event) override;

  void ProcessInitialState(uint64_t timestamp_ns, pid_t tid, char state_char);
  void visit(TaskNewtaskPerfEvent* event) override;
  void visit(SchedSwitchPerfEvent* event) override;
  void visit(SchedWakeupPerfEvent* event) override;
  void ProcessRemainingOpenStates(uint64_t timestamp_ns);

 private:
  static std::optional<orbit_grpc_protos::ThreadStateSlice::ThreadState> GetThreadStateFromChar(
      char c);
  static orbit_grpc_protos::ThreadStateSlice::ThreadState GetThreadStateFromBits(uint64_t bits);

  TracerListener* listener_ = nullptr;

  bool TidMatchesPidFilter(pid_t tid);
  std::optional<pid_t> GetPidOfTid(pid_t tid);
  static constexpr pid_t kPidFilterNoThreadState = -1;
  pid_t thread_state_pid_filter_ = kPidFilterNoThreadState;
  absl::flat_hash_map<pid_t, pid_t> tid_to_pid_association_;

  ContextSwitchManager switch_manager_;
  ThreadStateManager state_manager_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
