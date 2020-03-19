#include "UprobesUnwindingVisitor.h"

namespace LinuxTracing {

void UprobesUnwindingVisitor::visit(StackSamplePerfEvent* event) {
  CHECK(listener_ != nullptr);
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessSampledCallstack(event->GetTid(), *event);
  if (!full_callstack.empty()) {
    Callstack returned_callstack{
        event->GetTid(),
        CallstackFramesFromLibunwindstackFrames(full_callstack),
        event->GetTimestamp()};
    listener_->OnCallstack(returned_callstack);
  }
}

void UprobesUnwindingVisitor::visit(UprobesWithStackPerfEvent* event) {
  CHECK(listener_ != nullptr);

  // We are seeing that on thread migration, uprobe events can sometimes be
  // duplicated. The idea of the workaround is that for a given thread's
  // sequence of u(ret)probe events, two consecutive uprobe events must be
  // associated with decreasing stack pointers (nested function calls, stack
  // grows by decreasing stack pointer).  If an extra uprobe event is generated,
  // then the second uprobe event will be associated with a stack pointer that
  // is greater or equal to the previous stack pointer, and that's not normal.
  // In that situation, we discard the second uprobe event.

  // Duplicate uprobe detection.
  uint64_t uprobe_sp = event->GetRegisters()[PERF_REG_X86_SP];
  std::vector<uint64_t>& uprobe_sps = uprobe_sps_per_thread_[event->GetTid()];
  if (!uprobe_sps.empty()) {
    uint64_t last_uprobe_sp = uprobe_sps.back();
    uprobe_sps.pop_back();
    if (uprobe_sp >= last_uprobe_sp) {
      ERROR("MISSING URETPROBE OR DUPLICATE UPROBE DETECTED");
      return;
    }
  }
  uprobe_sps.push_back(uprobe_sp);

  function_call_manager_.ProcessUprobes(event->GetTid(),
                                        event->GetFunction()->VirtualAddress(),
                                        event->GetTimestamp());

  // Careful: UprobesWithStackPerfEvent* event ends up being moved from
  // LateUnwindCallstack's constructor.
  callstack_manager_.ProcessUprobesCallstack(event->GetTid(),
                                             std::move(*event));
}

void UprobesUnwindingVisitor::visit(UretprobesPerfEvent* event) {
  CHECK(listener_ != nullptr);

  // Duplicate uprobe detection.
  std::vector<uint64_t>& uprobe_sps = uprobe_sps_per_thread_[event->GetTid()];
  if (!uprobe_sps.empty()) {
    uprobe_sps.pop_back();
  }

  std::optional<FunctionCall> function_call =
      function_call_manager_.ProcessUretprobes(event->GetTid(),
                                               event->GetTimestamp());
  if (function_call.has_value()) {
    listener_->OnFunctionCall(function_call.value());
  }

  callstack_manager_.ProcessUretprobes(event->GetTid());
}

void UprobesUnwindingVisitor::visit(MapsPerfEvent* event) {
  callstack_manager_.ProcessMaps(event->GetMaps());
}

std::vector<CallstackFrame>
UprobesUnwindingVisitor::CallstackFramesFromLibunwindstackFrames(
    const std::vector<unwindstack::FrameData>& libunwindstack_frames) {
  std::vector<CallstackFrame> callstack_frames;
  callstack_frames.reserve(libunwindstack_frames.size());
  for (const unwindstack::FrameData& libunwindstack_frame :
       libunwindstack_frames) {
    callstack_frames.emplace_back(
        libunwindstack_frame.pc, libunwindstack_frame.function_name,
        libunwindstack_frame.function_offset, libunwindstack_frame.map_name);
  }
  return callstack_frames;
}

}  // namespace LinuxTracing
