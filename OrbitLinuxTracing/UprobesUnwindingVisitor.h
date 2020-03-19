#ifndef ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
#define ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include <stack>

#include "LibunwindstackUnwinder.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "UprobesCallstackManager.h"
#include "UprobesFunctionCallManager.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

// UprobesUnwindingVisitor visitor processes stack samples and
// uprobes/uretprobes records (as well as memory maps changes, to keep necessary
// unwinding information up-to-date), assuming they come in order.
// The reason for processing both in the same visitor is that, when entering a
// dynamically-instrumented function, the return address saved on the stack is
// hijacked by uretprobes. This causes unwinding of any (time-based) stack
// sample that falls inside such a function to stop at the first of such
// functions.
// In order to reconstruct such broken callstacks, UprobesCallstackManager keeps
// a stack, for every thread, of (broken) callstacks collected at the beginning
// of instrumented functions. When we have a callstack broken because of
// uretprobes we can then rebuild the missing part by joining together the parts
// on the stack of callstacks associated with that thread.
// TODO: Make this more robust to losing uprobes or uretprobes events (loss of
//  uretprobes events should be rare if they don't come with a stack sample).
//  Start by passing the function_address to ProcessUretprobes as well for a
//  comparison against the address of the uprobe on the stack.

class UprobesUnwindingVisitor : public PerfEventVisitor {
 public:
  explicit UprobesUnwindingVisitor(const std::string& initial_maps)
      : callstack_manager_{&unwinder_, initial_maps} {}

  UprobesUnwindingVisitor(const UprobesUnwindingVisitor&) = delete;
  UprobesUnwindingVisitor& operator=(const UprobesUnwindingVisitor&) = delete;

  UprobesUnwindingVisitor(UprobesUnwindingVisitor&&) = default;
  UprobesUnwindingVisitor& operator=(UprobesUnwindingVisitor&&) = default;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void visit(StackSamplePerfEvent* event) override;
  void visit(UprobesWithStackPerfEvent* event) override;
  void visit(UretprobesPerfEvent* event) override;
  void visit(MapsPerfEvent* event) override;

 private:
  UprobesFunctionCallManager function_call_manager_{};
  LibunwindstackUnwinder unwinder_{};
  UprobesCallstackManager<LibunwindstackUnwinder> callstack_manager_;

  TracerListener* listener_ = nullptr;

  static std::vector<CallstackFrame> CallstackFramesFromLibunwindstackFrames(
      const std::vector<unwindstack::FrameData>& libunwindstack_frames);

  absl::flat_hash_map<pid_t, std::vector<uint64_t>> uprobe_sps_per_thread_{};
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
