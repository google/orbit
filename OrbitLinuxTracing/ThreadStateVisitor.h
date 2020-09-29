// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
#define ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_

#include <OrbitLinuxTracing/TracerListener.h>

#include "PerfEventVisitor.h"

namespace LinuxTracing {

class ThreadStateVisitor : public PerfEventVisitor {
 public:
  void SetListener(TracerListener* listener) { listener_ = listener; }

  void ProcessInitialState(uint64_t timestamp_ns, pid_t tid, char state_char);
  void visit(TaskNewtaskPerfEvent* event) override;
  void visit(SchedSwitchPerfEvent* event) override;
  void visit(SchedWakeupPerfEvent* event) override;
  void ProcessRemainingOpenStates(uint64_t timestamp_ns);

 private:
  TracerListener* listener_ = nullptr;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
