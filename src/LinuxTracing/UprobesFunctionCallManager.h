// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
#define LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_

#include <absl/container/flat_hash_map.h>

#include <stack>

#include "OrbitBase/Logging.h"
#include "PerfEventRecords.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {

// Keeps a stack, for every thread, of the open uprobes and matches them with
// the uretprobes to produce FunctionCall objects.
class UprobesFunctionCallManager {
 public:
  UprobesFunctionCallManager() = default;

  UprobesFunctionCallManager(const UprobesFunctionCallManager&) = delete;
  UprobesFunctionCallManager& operator=(const UprobesFunctionCallManager&) = delete;

  UprobesFunctionCallManager(UprobesFunctionCallManager&&) = default;
  UprobesFunctionCallManager& operator=(UprobesFunctionCallManager&&) = default;

  void ProcessUprobes(pid_t tid, uint64_t function_id, uint64_t begin_timestamp,
                      std::optional<perf_event_sample_regs_user_sp_ip_arguments> regs) {
    auto& tid_uprobes_stack = tid_uprobes_stacks_[tid];
    tid_uprobes_stack.emplace(function_id, begin_timestamp, regs);
  }

  std::optional<orbit_grpc_protos::FunctionCall> ProcessUretprobes(
      pid_t pid, pid_t tid, uint64_t end_timestamp, std::optional<uint64_t> return_value) {
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
    function_call.set_function_id(tid_uprobe.function_id);
    function_call.set_duration_ns(end_timestamp - tid_uprobe.begin_timestamp);
    function_call.set_end_timestamp_ns(end_timestamp);
    function_call.set_depth(tid_uprobes_stack.size() - 1);
    if (return_value.has_value()) {
      function_call.set_return_value(return_value.value());
    }
    if (tid_uprobe.registers.has_value()) {
      function_call.add_registers(tid_uprobe.registers.value().di);
      function_call.add_registers(tid_uprobe.registers.value().si);
      function_call.add_registers(tid_uprobe.registers.value().dx);
      function_call.add_registers(tid_uprobe.registers.value().cx);
      function_call.add_registers(tid_uprobe.registers.value().r8);
      function_call.add_registers(tid_uprobe.registers.value().r9);
    }

    tid_uprobes_stack.pop();
    if (tid_uprobes_stack.empty()) {
      tid_uprobes_stacks_.erase(tid);
    }
    return function_call;
  }

 private:
  struct OpenUprobes {
    OpenUprobes(uint64_t function_id, uint64_t begin_timestamp,
                std::optional<perf_event_sample_regs_user_sp_ip_arguments> regs)
        : function_id{function_id}, begin_timestamp{begin_timestamp}, registers{regs} {}
    uint64_t function_id;
    uint64_t begin_timestamp;
    std::optional<perf_event_sample_regs_user_sp_ip_arguments> registers;
  };

  // This map keeps the stack of the dynamically-instrumented functions entered.
  absl::flat_hash_map<pid_t, std::stack<OpenUprobes, std::vector<OpenUprobes>>>
      tid_uprobes_stacks_{};
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
