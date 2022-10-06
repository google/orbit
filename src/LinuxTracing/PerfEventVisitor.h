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
  virtual void Visit(uint64_t /*event_timestamp*/, const ForkPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const ExitPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const StackSamplePerfEventData& /*event_data*/) {
  }
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const CallchainSamplePerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const UprobesPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UprobesWithStackPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UprobesWithArgumentsPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const UretprobesPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UretprobesWithReturnValuePerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UserSpaceFunctionEntryPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const UserSpaceFunctionExitPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const LostPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const DiscardedPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const MmapPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const TaskNewtaskPerfEventData& /*event_data*/) {
  }
  virtual void Visit(uint64_t /*event_timestamp*/, const TaskRenamePerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/, const SchedSwitchPerfEventData& /*event_data*/) {
  }
  virtual void Visit(uint64_t /*event_timestamp*/, const SchedWakeupPerfEventData& /*event_data*/) {
  }
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const SchedWakeupWithCallchainPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const SchedSwitchWithCallchainPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const SchedWakeupWithStackPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const SchedSwitchWithStackPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const AmdgpuCsIoctlPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const AmdgpuSchedRunJobPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const DmaFenceSignaledPerfEventData& /*event_data*/) {}
  virtual void Visit(uint64_t /*event_timestamp*/,
                     const GenericTracepointPerfEventData& /*event_data*/) {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_VISITOR_H_
