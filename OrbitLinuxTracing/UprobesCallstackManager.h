#ifndef ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_
#define ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_

#include "LibunwindstackUnwinder.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

class UprobesCallstackManager {
 public:
  UprobesCallstackManager() = default;

  UprobesCallstackManager(const UprobesCallstackManager&) = delete;
  UprobesCallstackManager& operator=(const UprobesCallstackManager&) = delete;

  UprobesCallstackManager(UprobesCallstackManager&&) = default;
  UprobesCallstackManager& operator=(UprobesCallstackManager&&) = default;

  std::vector<unwindstack::FrameData> ProcessUprobesCallstack(
      pid_t tid, const std::vector<unwindstack::FrameData>& callstack) {
    std::vector<std::vector<unwindstack::FrameData>>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    const std::vector<unwindstack::FrameData>& full_callstack =
        JoinCallstackWithPreviousUprobesCallstacks(callstack,
                                                   previous_callstacks);

    if (!callstack.empty()) {
      std::vector<unwindstack::FrameData> uprobes_callstack{};
      // Start from 1 to remove the instrumented function's entry.
      for (size_t i = 1; i < callstack.size(); ++i) {
        uprobes_callstack.push_back(callstack[i]);
      }
      if (uprobes_callstack.back().map_name == "[uprobes]") {
        // Remove the [uprobes] entry from the bottom.
        uprobes_callstack.pop_back();
      }
      previous_callstacks.push_back(std::move(uprobes_callstack));

    } else {
      // Put a placeholder indicating an error on the stack.
      previous_callstacks.emplace_back();
    }

    return full_callstack;
  }

  std::vector<unwindstack::FrameData> ProcessSampledCallstack(
      pid_t tid, const std::vector<unwindstack::FrameData>& callstack) {
    const std::vector<std::vector<unwindstack::FrameData>>&
        previous_callstacks = tid_uprobes_callstacks_stacks_[tid];
    const std::vector<unwindstack::FrameData>& full_callstack =
        JoinCallstackWithPreviousUprobesCallstacks(callstack,
                                                   previous_callstacks);
    return full_callstack;
  }

  void ProcessUretprobes(pid_t tid) {
    std::vector<std::vector<unwindstack::FrameData>>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    if (!previous_callstacks.empty()) {
      previous_callstacks.pop_back();
    }
    if (previous_callstacks.empty()) {
      tid_uprobes_callstacks_stacks_.erase(tid);
    }
  }

 private:
  // This map keeps, for every thread, the stack of callstacks collected when
  // entering a uprobes-instrumented function.
  absl::flat_hash_map<pid_t, std::vector<std::vector<unwindstack::FrameData>>>
      tid_uprobes_callstacks_stacks_{};

  static std::vector<unwindstack::FrameData>
  JoinCallstackWithPreviousUprobesCallstacks(
      const std::vector<unwindstack::FrameData>& this_callstack,
      const std::vector<std::vector<unwindstack::FrameData>>&
          previous_callstacks) {
    if (this_callstack.empty()) {
      // This callstack is an unwinding failure.
      return {};
    }

    if (this_callstack.back().map_name != "[uprobes]") {
      // This callstack is already complete.
      return this_callstack;
    }

    for (auto previous_callstack = previous_callstacks.rbegin();
         previous_callstack != previous_callstacks.rend();
         ++previous_callstack) {
      if (previous_callstack->empty()) {
        // A previous callstack was an unwinding failure, hence unfortunately
        // this is a failure as well.
        return {};
      }
    }

    std::vector<unwindstack::FrameData> full_callstack = this_callstack;
    full_callstack.pop_back();  // Remove [uprobes] entry.

    // Append the previous callstacks, from the most recent.
    for (auto previous_callstack = previous_callstacks.rbegin();
         previous_callstack != previous_callstacks.rend();
         ++previous_callstack) {
      for (const auto& frame : *previous_callstack) {
        full_callstack.push_back(frame);
      }
    }

    return full_callstack;
  }
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_
