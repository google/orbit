// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
#define ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_

#include <OrbitLinuxTracing/TracerListener.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "ThreadStateManager.h"
#include "capture.pb.h"

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
  static std::optional<orbit_grpc_protos::ThreadStateSlice::ThreadState> GetThreadStateFromChar(
      char c);
  static orbit_grpc_protos::ThreadStateSlice::ThreadState GetThreadStateFromBits(uint64_t bits);

  TracerListener* listener_ = nullptr;
  ThreadStateManager state_manager_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_THREAD_STATE_VISITOR_H_
