// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
#define ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_

#include <OrbitBase/Logging.h>

#include <stack>

#include "PerfEventRecords.h"
#include "absl/container/flat_hash_map.h"
#include "capture.pb.h"

namespace LinuxTracing {

// Keeps a stack, for every thread, of the open uprobes and matches them with
// the uretprobes to produce FunctionCall objects.
class UprobesFunctionCallManager {
 public:
  UprobesFunctionCallManager() = default;

  UprobesFunctionCallManager(const UprobesFunctionCallManager&) = delete;
  UprobesFunctionCallManager& operator=(const UprobesFunctionCallManager&) = delete;

  UprobesFunctionCallManager(UprobesFunctionCallManager&&) = default;
  UprobesFunctionCallManager& operator=(UprobesFunctionCallManager&&) = default;

  void ProcessUprobes(pid_t tid, uint64_t function_address, uint64_t begin_timestamp,
                      const perf_event_sample_regs_user_sp_ip_arguments& regs) {
    auto& tid_uprobes_stack = tid_uprobes_stacks_[tid];
    tid_uprobes_stack.emplace(function_address, begin_timestamp, regs);
  }

  std::optional<orbit_grpc_protos::FunctionCall> ProcessUretprobes(pid_t pid, pid_t tid,
                                                                   uint64_t end_timestamp,
                                                                   uint64_t return_value) {
    if (!tid_uprobes_stacks_.contains(tid)) {
      return std::optional<orbit_grpc_protos::FunctionCall>{};
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);

    // As we erase the stack for this thread as soon as it becomes empty.
    CHECK(!tid_uprobes_stack.empty());
    auto& tid_uprobe = tid_uprobes_stack.top();

    orbit_grpc_protos::FunctionCall function_call;
    function_call.set_pid(pid);
    function_call.set_tid(tid);
    function_call.set_absolute_address(tid_uprobe.function_address);
    function_call.set_begin_timestamp_ns(tid_uprobe.begin_timestamp);
    function_call.set_end_timestamp_ns(end_timestamp);
    function_call.set_depth(tid_uprobes_stack.size() - 1);
    function_call.set_return_value(return_value);
    function_call.add_registers(tid_uprobe.registers.di);
    function_call.add_registers(tid_uprobe.registers.si);
    function_call.add_registers(tid_uprobe.registers.dx);
    function_call.add_registers(tid_uprobe.registers.cx);
    function_call.add_registers(tid_uprobe.registers.r8);
    function_call.add_registers(tid_uprobe.registers.r9);

    tid_uprobes_stack.pop();
    if (tid_uprobes_stack.empty()) {
      tid_uprobes_stacks_.erase(tid);
    }
    return function_call;
  }

 private:
  struct OpenUprobes {
    OpenUprobes(uint64_t function_address, uint64_t begin_timestamp,
                const perf_event_sample_regs_user_sp_ip_arguments& regs)
        : function_address{function_address}, begin_timestamp{begin_timestamp}, registers(regs) {}
    uint64_t function_address;
    uint64_t begin_timestamp;
    perf_event_sample_regs_user_sp_ip_arguments registers;
  };

  // This map keeps the stack of the dynamically-instrumented functions entered.
  absl::flat_hash_map<pid_t, std::stack<OpenUprobes, std::vector<OpenUprobes>>>
      tid_uprobes_stacks_{};
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
