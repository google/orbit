// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_VISITOR_H_
#define LINUX_TRACING_PERF_EVENT_VISITOR_H_

#include "PerfEvent.h"

namespace orbit_linux_tracing {

// Keep this class in sync with the types of `std::variant PerfEvent::data` in PerfEvent.h.
class PerfEventVisitor {
 public:
  virtual ~PerfEventVisitor() = default;
  virtual void Visit(uint64_t /*event_timestamp*/, const ForkPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const ExitPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const StackSamplePerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const CallchainSamplePerfEvent& /*event_data*/) {
  }
  virtual void Visit(uint64_t /*event_timestamp*/, const UprobesPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UprobesWithArgumentsPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const UretprobesPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UretprobesWithReturnValuePerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const LostPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const DiscardedPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const MmapPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const TaskNewtaskPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const TaskRenamePerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const SchedSwitchPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const SchedWakeupPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const AmdgpuCsIoctlPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const AmdgpuSchedRunJobPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const DmaFenceSignaledPerfEvent& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const GenericTracepointPerfEvent& /*event_data*/) {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_VISITOR_H_
