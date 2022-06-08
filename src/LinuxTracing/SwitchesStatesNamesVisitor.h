// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_THREAD_STATE_VISITOR_H_
#define LINUX_TRACING_THREAD_STATE_VISITOR_H_

#include <absl/container/flat_hash_map.h>
#include <sys/types.h>

#include <atomic>
#include <cstdint>
#include <optional>

#include "ContextSwitchManager.h"
#include "GrpcProtos/capture.pb.h"
#include "LinuxTracing/TracerListener.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "ThreadStateManager.h"

namespace orbit_linux_tracing {

// This PerfEventVisitor visits PerfEvents associated with scheduling slices and thread states,
// processes them using ContextSwitchManager and ThreadStateManager, and passes the results to the
// specified TracerListener.
// As for some of these events the process id is not available, but only the thread id, this class
// also keeps the association between tids and pids system wide. The initial association extracted
// from the proc filesystem is passed by calling ProcessInitialTidToPidAssociation for each thread,
// and is updated with ForkPerfEvents (and also ExitPerfEvents, see Visit(const ExitPerfEvent&)).
// For thread states, we are able to collect partial slices at the beginning and at the end of the
// capture, hence the ProcessInitialState and ProcessRemainingOpenStates methods.
// Also, we only process thread states of the processes with pids specified with
// SetThreadStatePidFilters (so that we can collect thread states only for the processes we are
// profiling). For this we also need the system-wide association between tids and pids.
class SwitchesStatesNamesVisitor : public PerfEventVisitor {
 public:
  explicit SwitchesStatesNamesVisitor(TracerListener* listener) : listener_{listener} {
    ORBIT_CHECK(listener_ != nullptr);
  }

  void SetThreadStateCounter(std::atomic<uint64_t>* thread_state_counter) {
    thread_state_counter_ = thread_state_counter;
  }

  void SetProduceSchedulingSlices(bool produce_scheduling_slices) {
    produce_scheduling_slices_ = produce_scheduling_slices;
  }
  void SetThreadStatePidFilters(std::set<pid_t> pids) {
    thread_state_pid_filters_ = {pids.begin(), pids.end()};
  }

  void ProcessInitialTidToPidAssociation(pid_t tid, pid_t pid);
  void Visit(uint64_t event_timestamp, const ForkPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const ExitPerfEventData& event_data) override;

  void ProcessInitialState(uint64_t timestamp_ns, pid_t tid, char state_char);
  void Visit(uint64_t event_timestamp, const TaskNewtaskPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const SchedSwitchPerfEventData& event_data) override;
  void Visit(uint64_t timestamp, const SchedWakeupPerfEventData& event_data) override;
  void ProcessRemainingOpenStates(uint64_t timestamp_ns);

  void Visit(uint64_t event_timestamp, const TaskRenamePerfEventData& event_data) override;

 private:
  static std::optional<orbit_grpc_protos::ThreadStateSlice::ThreadState> GetThreadStateFromChar(
      char c);
  static orbit_grpc_protos::ThreadStateSlice::ThreadState GetThreadStateFromBits(uint64_t bits);

  TracerListener* listener_;
  std::atomic<uint64_t>* thread_state_counter_ = nullptr;

  bool produce_scheduling_slices_ = false;

  bool TidMatchesPidFilter(pid_t tid);
  std::optional<pid_t> GetPidOfTid(pid_t tid);
  std::vector<pid_t> thread_state_pid_filters_;
  absl::flat_hash_map<pid_t, pid_t> tid_to_pid_association_;

  ContextSwitchManager switch_manager_;
  ThreadStateManager state_manager_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_THREAD_STATE_VISITOR_H_
