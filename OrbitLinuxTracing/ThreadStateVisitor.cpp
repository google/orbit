// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadStateVisitor.h"

namespace LinuxTracing {

void ThreadStateVisitor::ProcessInitialState(uint64_t /*timestamp_ns*/, pid_t tid,
                                             char state_char) {
  LOG("ProcessInitialState(tid=%d, state_char=%c)", tid, state_char);
}

void ThreadStateVisitor::visit(TaskNewtaskPerfEvent* event) {
  CHECK(listener_ != nullptr);
  LOG("task/task_newtask | tid: %d", event->GetTid());
}

void ThreadStateVisitor::visit(SchedSwitchPerfEvent* event) {
  CHECK(listener_ != nullptr);
  LOG("sched/sched_switch | prev_comm: %s, prev_pid: %d, prev_state: %#lx; next_comm: %s, "
      "next_pid: %d",
      event->GetPrevComm(), event->GetPrevTid(), event->GetPrevState(), event->GetNextComm(),
      event->GetNextTid());
}

void ThreadStateVisitor::visit(SchedWakeupPerfEvent* event) {
  CHECK(listener_ != nullptr);
  LOG("sched/sched_wakeup | (waker)pid: %d, (waker)tid: %d; (woken)tid: %d", event->GetWakerPid(),
      event->GetWakerTid(), event->GetWokenTid());
}

void ThreadStateVisitor::ProcessRemainingOpenStates(uint64_t /*timestamp_ns*/) {
  CHECK(listener_ != nullptr);
  LOG("ProcessRemainingOpenStates");
}

}  // namespace LinuxTracing
