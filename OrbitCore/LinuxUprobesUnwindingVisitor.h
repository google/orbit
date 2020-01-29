#ifndef ORBIT_CORE_LINUX_UPROBES_UNWINDING_VISITOR_H_
#define ORBIT_CORE_LINUX_UPROBES_UNWINDING_VISITOR_H_

#include "LibunwindstackUnwinder.h"
#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "ScopeTimer.h"

// This visitor processes stack samples and uprobes/uretprobes records (as well
// as memory maps changes, to keep necessary unwinding information up-to-date),
// assuming they come in order.
// The reason for processing both in the same visitor is that, when entering a
// dynamically-instrumented function, the return address saved on the stack is
// hijacked by uretprobes. This causes unwinding of any (time-based) stack
// sample that falls inside such a function to stop at the first of such
// functions.
// In order to reconstruct such broken callstacks, we keep a stack, for every
// thread, of (broken) callstacks collected at the beginning of instrumented
// functions. When we have a callstack broken because of uretprobes we can then
// rebuild the missing part by joining together the parts on the stack of
// callstacks associated with that thread.
class LinuxUprobesUnwindingVisitor : public LinuxPerfEventVisitor {
 public:
  explicit LinuxUprobesUnwindingVisitor(pid_t pid, const std::string& maps)
      : pid_{pid} {
    unwinder_.SetMaps(maps);
  }

  LinuxUprobesUnwindingVisitor(const LinuxUprobesUnwindingVisitor&) = delete;
  LinuxUprobesUnwindingVisitor& operator=(const LinuxUprobesUnwindingVisitor&) =
      delete;

  LinuxUprobesUnwindingVisitor(LinuxUprobesUnwindingVisitor&&) = default;
  LinuxUprobesUnwindingVisitor& operator=(LinuxUprobesUnwindingVisitor&&) =
      default;

  void visit(LinuxStackSampleEvent* event) override;
  void visit(LinuxUprobeEventWithStack* event) override;
  void visit(LinuxUretprobeEventWithStack* event) override;
  void visit(LinuxMapsEvent* event) override;

 private:
  pid_t pid_;
  // This map keeps the stack of the dynamically-instrumented functions entered.
  std::unordered_map<pid_t, std::vector<Timer>> tid_timer_stacks_{};

  LibunwindstackUnwinder unwinder_{};
  // This map keeps, for every thread, the stack of callstacks collected when
  // entering a uprobes-instrumented function.
  std::unordered_map<pid_t, std::vector<std::vector<unwindstack::FrameData>>>
      tid_uprobes_callstacks_stacks_{};

  static std::vector<unwindstack::FrameData>
  JoinCallstackWithPreviousUprobesCallstacks(
      const std::vector<unwindstack::FrameData>& this_callstack,
      const std::vector<std::vector<unwindstack::FrameData>>&
          previous_callstacks);
  static void HandleCallstack(
      pid_t tid, uint64_t timestamp,
      const std::vector<unwindstack::FrameData>& frames);
};

#endif  // ORBIT_CORE_LINUX_UPROBES_UNWINDING_VISITOR_H_
