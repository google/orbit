// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
#define LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_

#include <absl/container/flat_hash_map.h>

#include <stack>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "PerfEventRecords.h"

namespace orbit_linux_tracing {

// Keeps a stack, for every thread, of the dynamically instrumented functions that have been entered
// (e.g., open uprobes) and matches them with the exits from those functions (e.g., uretprobes) to
// produce FunctionCall objects.
class UprobesFunctionCallManager {
 public:
  UprobesFunctionCallManager() = default;

  UprobesFunctionCallManager(const UprobesFunctionCallManager&) = delete;
  UprobesFunctionCallManager& operator=(const UprobesFunctionCallManager&) = delete;

  UprobesFunctionCallManager(UprobesFunctionCallManager&&) = default;
  UprobesFunctionCallManager& operator=(UprobesFunctionCallManager&&) = default;

  void ProcessFunctionEntry(pid_t tid, uint64_t function_id, uint64_t begin_timestamp,
                            std::optional<perf_event_sample_regs_user_sp_ip_arguments> regs) {
    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_[tid];
    stack_of_open_functions.emplace_back(function_id, begin_timestamp, regs);
  }

  std::optional<orbit_grpc_protos::FunctionCall> ProcessFunctionExit(
      pid_t pid, pid_t tid, uint64_t end_timestamp, std::optional<uint64_t> return_value) {
    if (!tid_to_stack_of_open_functions_.contains(tid)) {
      return std::nullopt;
    }

    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_.at(tid);

    // As we erase the stack for this thread as soon as it becomes empty.
    ORBIT_CHECK(!stack_of_open_functions.empty());
    OpenFunction& open_function = stack_of_open_functions.back();

    orbit_grpc_protos::FunctionCall function_call;
    function_call.set_pid(pid);
    function_call.set_tid(tid);
    function_call.set_function_id(open_function.function_id);
    function_call.set_duration_ns(end_timestamp - open_function.begin_timestamp);
    function_call.set_end_timestamp_ns(end_timestamp);
    function_call.set_depth(static_cast<int32_t>(stack_of_open_functions.size() - 1));
    if (return_value.has_value()) {
      function_call.set_return_value(return_value.value());
    }
    if (open_function.registers.has_value()) {
      function_call.add_registers(open_function.registers.value().di);
      function_call.add_registers(open_function.registers.value().si);
      function_call.add_registers(open_function.registers.value().dx);
      function_call.add_registers(open_function.registers.value().cx);
      function_call.add_registers(open_function.registers.value().r8);
      function_call.add_registers(open_function.registers.value().r9);
    }

    stack_of_open_functions.pop_back();
    if (stack_of_open_functions.empty()) {
      tid_to_stack_of_open_functions_.erase(tid);
    }
    return function_call;
  }

 private:
  struct OpenFunction {
    OpenFunction(uint64_t function_id, uint64_t begin_timestamp,
                 std::optional<perf_event_sample_regs_user_sp_ip_arguments> regs)
        : function_id{function_id}, begin_timestamp{begin_timestamp}, registers{regs} {}
    uint64_t function_id;
    uint64_t begin_timestamp;
    std::optional<perf_event_sample_regs_user_sp_ip_arguments> registers;
  };

  // This map keeps the stack of the dynamically-instrumented functions entered.
  absl::flat_hash_map<pid_t, std::vector<OpenFunction>> tid_to_stack_of_open_functions_{};
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
