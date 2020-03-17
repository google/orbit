#ifndef ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
#define ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_

#include <OrbitBase/Logging.h>
#include <OrbitLinuxTracing/Events.h>

#include <stack>

#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

class UprobesFunctionCallManager {
 public:
  UprobesFunctionCallManager() = default;

  UprobesFunctionCallManager(const UprobesFunctionCallManager&) = delete;
  UprobesFunctionCallManager& operator=(const UprobesFunctionCallManager&) =
      delete;

  UprobesFunctionCallManager(UprobesFunctionCallManager&&) = default;
  UprobesFunctionCallManager& operator=(UprobesFunctionCallManager&&) = default;

  void ProcessUprobes(pid_t tid, uint64_t function_address,
                      uint64_t begin_timestamp) {
    auto& tid_timer_stack = tid_timer_stacks_[tid];
    tid_timer_stack.emplace(function_address, begin_timestamp);
  }

  std::optional<FunctionCall> ProcessUretprobes(pid_t tid,
                                                uint64_t end_timestamp) {
    if (tid_timer_stacks_.count(tid) == 0) {
      return std::optional<FunctionCall>{};
    }

    auto& tid_timer_stack = tid_timer_stacks_.at(tid);

    // As we erase the stack for this thread as soon as it becomes empty.
    CHECK(!tid_timer_stack.empty());

    auto function_call = std::make_optional<FunctionCall>(
        tid, tid_timer_stack.top().function_address,
        tid_timer_stack.top().begin_timestamp, end_timestamp,
        tid_timer_stack.size() - 1);
    tid_timer_stack.pop();
    if (tid_timer_stack.empty()) {
      tid_timer_stacks_.erase(tid);
    }
    return function_call;
  }

 private:
  struct OpenUprobes {
    OpenUprobes(uint64_t function_address, uint64_t begin_timestamp)
        : function_address(function_address),
          begin_timestamp(begin_timestamp) {}
    uint64_t function_address;
    uint64_t begin_timestamp;
  };

  // This map keeps the stack of the dynamically-instrumented functions entered.
  absl::flat_hash_map<pid_t, std::stack<OpenUprobes, std::vector<OpenUprobes>>>
      tid_timer_stacks_{};
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_FUNCTION_CALL_MANAGER_H_
