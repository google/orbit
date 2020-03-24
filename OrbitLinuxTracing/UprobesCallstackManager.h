#ifndef ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_
#define ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_

#include "LibunwindstackUnwinder.h"
#include "PerfEvent.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

// LateUnwindCallstack holds either a UprobesWithStackPerfEvent and the snapshot
// of the maps needed to unwind it, or the already unwound callstack.
class LateUnwindCallstack {
 public:
  // uprobes_event needs to be moved from because a copy would be expensive.
  explicit LateUnwindCallstack(UprobesWithStackPerfEvent&& uprobes_event,
                               std::shared_ptr<unwindstack::BufferMaps> maps)
      : uprobes_event_{std::make_unique<UprobesWithStackPerfEvent>(
            std::move(uprobes_event))},
        maps_{std::move(maps)} {}

  bool IsUnwound() const { return uprobes_event_ == nullptr; }

  UprobesWithStackPerfEvent* GetUprobesEvent() const {
    return uprobes_event_.get();
  }

  unwindstack::BufferMaps* GetMaps() const { return maps_.get(); }

  void SetCallstack(std::vector<unwindstack::FrameData> callstack) {
    uprobes_event_ = nullptr;
    maps_ = nullptr;
    callstack_ = std::move(callstack);
  }

  const std::vector<unwindstack::FrameData>& GetCallstack() {
    return callstack_;
  }

  bool IsCallstackValid() const { return !callstack_.empty(); }

 private:
  std::unique_ptr<UprobesWithStackPerfEvent> uprobes_event_;
  std::shared_ptr<unwindstack::BufferMaps> maps_;
  std::vector<unwindstack::FrameData> callstack_{};
};

// UprobesCallstackManager is a class template to simplify testing, so that we
// can pass a mock unwinder.
template <typename UnwinderT>
class UprobesCallstackManager {
 public:
  UprobesCallstackManager(UnwinderT* unwinder, const std::string& initial_maps)
      : unwinder_{unwinder} {
    ProcessMaps(initial_maps);
  }

  UprobesCallstackManager(const UprobesCallstackManager&) = delete;
  UprobesCallstackManager& operator=(const UprobesCallstackManager&) = delete;

  UprobesCallstackManager(UprobesCallstackManager&&) = default;
  UprobesCallstackManager& operator=(UprobesCallstackManager&&) = default;

  void ProcessMaps(const std::string& maps_buffer) {
    current_maps_ = LibunwindstackUnwinder::ParseMaps(maps_buffer);
  }

  void ProcessUprobesCallstack(pid_t tid,
                               UprobesWithStackPerfEvent&& uprobes_event) {
    std::vector<LateUnwindCallstack>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    previous_callstacks.emplace_back(std::move(uprobes_event), current_maps_);
  }

  std::vector<unwindstack::FrameData> ProcessSampledCallstack(
      pid_t tid, const StackSamplePerfEvent& sample_event) {
    std::vector<unwindstack::FrameData> this_callstack = unwinder_->Unwind(
        current_maps_.get(), sample_event.GetRegisters(),
        sample_event.GetStackData(), sample_event.GetStackSize());
    if (this_callstack.empty()) {
      return {};
    }
    UnwindPreviousUprobesCallstacks(tid);
    const std::vector<unwindstack::FrameData>& full_callstack =
        JoinCallstackWithPreviousUprobesCallstacks(tid, this_callstack);
    return full_callstack;
  }

  void ProcessUretprobes(pid_t tid) {
    std::vector<LateUnwindCallstack>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    if (!previous_callstacks.empty()) {
      previous_callstacks.pop_back();
    }
    if (previous_callstacks.empty()) {
      tid_uprobes_callstacks_stacks_.erase(tid);
    }
  }

 private:
  UnwinderT* unwinder_;
  std::shared_ptr<unwindstack::BufferMaps> current_maps_ = nullptr;
  // This map keeps, for every thread, the stack of callstacks collected when
  // entering a uprobes-instrumented function.
  absl::flat_hash_map<pid_t, std::vector<LateUnwindCallstack>>
      tid_uprobes_callstacks_stacks_{};

  void UnwindPreviousUprobesCallstacks(pid_t tid) {
    std::vector<LateUnwindCallstack>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    for (LateUnwindCallstack& late_unwind_callstack : previous_callstacks) {
      if (!late_unwind_callstack.IsUnwound()) {
        const std::vector<unwindstack::FrameData>& callstack =
            unwinder_->Unwind(
                late_unwind_callstack.GetMaps(),
                late_unwind_callstack.GetUprobesEvent()->GetRegisters(),
                late_unwind_callstack.GetUprobesEvent()->GetStackData(),
                late_unwind_callstack.GetUprobesEvent()->GetStackSize());
        late_unwind_callstack.SetCallstack(callstack);
      }
      if (!late_unwind_callstack.IsCallstackValid()) {
        // There is an unwinding error on the stack, no point in unwinding more.
        break;
      }
    }
  }

  std::vector<unwindstack::FrameData>
  JoinCallstackWithPreviousUprobesCallstacks(
      pid_t tid, const std::vector<unwindstack::FrameData>& this_callstack) {
    if (this_callstack.empty()) {
      // This callstack is an unwinding failure.
      return {};
    }

    if (this_callstack.back().map_name != "[uprobes]") {
      // If the outermost frame is not a uprobes frame, then this_callstack is
      // already complete.
      return this_callstack;
    }

    std::vector<LateUnwindCallstack>& previous_callstacks =
        tid_uprobes_callstacks_stacks_[tid];
    for (const LateUnwindCallstack& previous_callstack : previous_callstacks) {
      CHECK(previous_callstack.IsUnwound());
      if (!previous_callstack.IsCallstackValid()) {
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
      CHECK(previous_callstack->IsUnwound());
      CHECK(previous_callstack->IsCallstackValid());
      // Start from 1 to remove the instrumented function's entry.
      for (size_t i = 1; i < previous_callstack->GetCallstack().size(); ++i) {
        full_callstack.push_back(previous_callstack->GetCallstack()[i]);
      }
      if (full_callstack.back().map_name == "[uprobes]") {
        // Remove the [uprobes] entry that was at the bottom of
        // previous_callstack if previous_callstack wasn't the outermost one.
        full_callstack.pop_back();
      }
    }

    return full_callstack;
  }
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_CALLSTACK_MANAGER_H_
