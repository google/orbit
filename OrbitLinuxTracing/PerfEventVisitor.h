// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_VISITOR_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_VISITOR_H_

#include "PerfEvent.h"

namespace LinuxTracing {

// Keep this class in sync with the hierarchy of PerfEvent in PerfEvent.h.
class PerfEventVisitor {
 public:
  virtual ~PerfEventVisitor() = default;
  virtual void visit(ForkPerfEvent*) {}
  virtual void visit(ExitPerfEvent*) {}
  virtual void visit(ContextSwitchPerfEvent*) {}
  virtual void visit(SystemWideContextSwitchPerfEvent*) {}
  virtual void visit(StackSamplePerfEvent*) {}
  virtual void visit(CallchainSamplePerfEvent*) {}
  virtual void visit(UprobesPerfEvent*) {}
  virtual void visit(UretprobesPerfEvent*) {}
  virtual void visit(LostPerfEvent*) {}
  virtual void visit(MapsPerfEvent*) {}
  virtual void visit(TaskRenamePerfEvent*) {}
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_VISITOR_H_
