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
  virtual void Visit(const ForkPerfEvent& /*event*/) {}
  virtual void Visit(const ExitPerfEvent& /*event*/) {}
  virtual void Visit(const StackSamplePerfEvent& /*event*/) {}
  virtual void Visit(const CallchainSamplePerfEvent& /*event*/) {}
  virtual void Visit(const UprobesPerfEvent& /*event*/) {}
  virtual void Visit(const UprobesWithArgumentsPerfEvent& /*event*/) {}
  virtual void Visit(const UretprobesPerfEvent& /*event*/) {}
  virtual void Visit(const UretprobesWithReturnValuePerfEvent& /*event*/) {}
  virtual void Visit(const LostPerfEvent& /*event*/) {}
  virtual void Visit(const DiscardedPerfEvent& /*event*/) {}
  virtual void Visit(const MmapPerfEvent& /*event*/) {}
  virtual void Visit(const TaskNewtaskPerfEvent& /*event*/) {}
  virtual void Visit(const TaskRenamePerfEvent& /*event*/) {}
  virtual void Visit(const SchedSwitchPerfEvent& /*event*/) {}
  virtual void Visit(const SchedWakeupPerfEvent& /*event*/) {}
  virtual void Visit(const AmdgpuCsIoctlPerfEvent& /*event*/) {}
  virtual void Visit(const AmdgpuSchedRunJobPerfEvent& /*event*/) {}
  virtual void Visit(const DmaFenceSignaledPerfEvent& /*event*/) {}
  virtual void Visit(const GenericTracepointPerfEvent& /*event*/) {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_VISITOR_H_
