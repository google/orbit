// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_VISITOR_H_
#define LINUX_TRACING_PERF_EVENT_VISITOR_H_

#include "PerfEvent.h"

namespace orbit_linux_tracing {

// Keep this class in sync with the hierarchy of PerfEvent in PerfEvent.h.
class PerfEventVisitor {
 public:
  virtual ~PerfEventVisitor() = default;
  virtual void Visit(ForkPerfEvent* /*event*/) {}
  virtual void Visit(ExitPerfEvent* /*event*/) {}
  virtual void Visit(ContextSwitchPerfEvent* /*event*/) {}
  virtual void Visit(SystemWideContextSwitchPerfEvent* /*event*/) {}
  virtual void Visit(StackSamplePerfEvent* /*event*/) {}
  virtual void Visit(CallchainSamplePerfEvent* /*event*/) {}
  virtual void Visit(UprobesPerfEvent* /*event*/) {}
  virtual void Visit(UprobesWithArgumentsPerfEvent* /*event*/) {}
  virtual void Visit(UretprobesPerfEvent* /*event*/) {}
  virtual void Visit(UretprobesWithReturnValuePerfEvent* /*event*/) {}
  virtual void Visit(LostPerfEvent* /*event*/) {}
  virtual void Visit(DiscardedPerfEvent* /*event*/) {}
  virtual void Visit(MmapPerfEvent* /*event*/) {}
  virtual void Visit(TaskNewtaskPerfEvent* /*event*/) {}
  virtual void Visit(TaskRenamePerfEvent* /*event*/) {}
  virtual void Visit(SchedSwitchPerfEvent* /*event*/) {}
  virtual void Visit(SchedWakeupPerfEvent* /*event*/) {}
  virtual void Visit(AmdgpuCsIoctlPerfEvent* /*event*/) {}
  virtual void Visit(AmdgpuSchedRunJobPerfEvent* /*event*/) {}
  virtual void Visit(DmaFenceSignaledPerfEvent* /*event*/) {}
  virtual void Visit(GenericTracepointPerfEvent* /*event*/) {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_VISITOR_H_
