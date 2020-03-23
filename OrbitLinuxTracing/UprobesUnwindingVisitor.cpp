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

  // We are seeing that, on thread migration, uprobe events can sometimes be
  // duplicated: the duplicate uprobe event will have the same stack pointer and
  // instruction pointer as the previous uprobe, but different cpu. In that
  // situation, we discard the second uprobe event.
  // We also discard a uprobe event in the general case of strictly-increasing
  // stack pointers, as for a given thread's sequence of u(ret)probe events, two
  // consecutive uprobe events must be associated with non-increasing stack
  // pointers (the stack grows towards lower addresses).

  // Duplicate uprobe detection.
  uint64_t uprobe_sp = event->GetRegisters()[PERF_REG_X86_SP];
  uint64_t uprobe_ip = event->GetRegisters()[PERF_REG_X86_IP];
  uint32_t uprobe_cpu = event->GetCpu();
  std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& uprobe_sps_ips_cpus =
      uprobe_sps_ips_cpus_per_thread_[event->GetTid()];
  if (!uprobe_sps_ips_cpus.empty()) {
    uint64_t last_uprobe_sp = std::get<0>(uprobe_sps_ips_cpus.back());
    uint64_t last_uprobe_ip = std::get<1>(uprobe_sps_ips_cpus.back());
    uint32_t last_uprobe_cpu = std::get<2>(uprobe_sps_ips_cpus.back());
    uprobe_sps_ips_cpus.pop_back();
    if (uprobe_sp > last_uprobe_sp) {
      ERROR("MISSING URETPROBE OR DUPLICATE UPROBE");
      return;
    } else if (uprobe_sp == last_uprobe_sp && uprobe_ip == last_uprobe_ip &&
               uprobe_cpu != last_uprobe_cpu) {
      ERROR("Duplicate uprobe on thread migration");
      return;
    }
  }
  uprobe_sps_ips_cpus.emplace_back(uprobe_sp, uprobe_ip, uprobe_cpu);

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
  std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& uprobe_sps_ips_cpus =
      uprobe_sps_ips_cpus_per_thread_[event->GetTid()];
  if (!uprobe_sps_ips_cpus.empty()) {
    uprobe_sps_ips_cpus.pop_back();
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
