#include "UprobesUnwindingVisitor.h"

namespace LinuxTracing {

void UprobesFunctionCallManager::ProcessUprobes(pid_t tid,
                                                uint64_t function_address,
                                                uint64_t begin_timestamp) {
  auto& tid_timer_stack = tid_timer_stacks_[tid];
  tid_timer_stack.emplace(function_address, begin_timestamp);
}

std::optional<FunctionCall> UprobesFunctionCallManager::ProcessUretprobes(
    pid_t tid, uint64_t end_timestamp) {
  if (tid_timer_stacks_.count(tid) == 0) {
    return std::optional<FunctionCall>{};
  }

  auto& tid_timer_stack = tid_timer_stacks_.at(tid);

  // As we erase the stack for this thread as soon as it becomes empty.
  assert(!tid_timer_stack.empty());

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

std::vector<unwindstack::FrameData>
UprobesCallstackManager::JoinCallstackWithPreviousUprobesCallstacks(
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
       previous_callstack != previous_callstacks.rend(); ++previous_callstack) {
    if (previous_callstack->empty()) {
      // A previous callstack was an unwinding failure, hence unfortunately this
      // is a failure as well.
      return {};
    }
  }

  std::vector<unwindstack::FrameData> full_callstack = this_callstack;
  full_callstack.pop_back();  // Remove [uprobes] entry.

  // Append the previous callstacks, from the most recent.
  for (auto previous_callstack = previous_callstacks.rbegin();
       previous_callstack != previous_callstacks.rend(); ++previous_callstack) {
    for (const auto& frame : *previous_callstack) {
      full_callstack.push_back(frame);
    }
  }

  return full_callstack;
}

std::vector<unwindstack::FrameData>
UprobesCallstackManager::ProcessUprobesCallstack(
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

std::vector<unwindstack::FrameData>
UprobesCallstackManager::ProcessSampledCallstack(
    pid_t tid, const std::vector<unwindstack::FrameData>& callstack) {
  const std::vector<std::vector<unwindstack::FrameData>>& previous_callstacks =
      tid_uprobes_callstacks_stacks_[tid];
  const std::vector<unwindstack::FrameData>& full_callstack =
      JoinCallstackWithPreviousUprobesCallstacks(callstack,
                                                 previous_callstacks);
  return full_callstack;
}

void UprobesCallstackManager::ProcessUretprobes(pid_t tid) {
  std::vector<std::vector<unwindstack::FrameData>>& previous_callstacks =
      tid_uprobes_callstacks_stacks_[tid];
  if (!previous_callstacks.empty()) {
    previous_callstacks.pop_back();
  }
  if (previous_callstacks.empty()) {
    tid_uprobes_callstacks_stacks_.erase(tid);
  }
}

void UprobesUnwindingVisitor::visit(StackSamplePerfEvent* event) {
  const std::vector<unwindstack::FrameData>& callstack = unwinder_.Unwind(
      event->Registers(), event->StackDump(), event->StackSize());
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessSampledCallstack(event->TID(), callstack);
  if (!full_callstack.empty() && listener_ != nullptr) {
    Callstack returned_callstack{
        event->TID(), CallstackFramesFromLibunwindstackFrames(full_callstack),
        event->Timestamp()};
    listener_->OnCallstack(returned_callstack);
  }
}

void UprobesUnwindingVisitor::visit(UprobesWithStackPerfEvent* event) {
  function_call_manager_.ProcessUprobes(
      event->TID(), event->GetFunction()->VirtualAddress(), event->Timestamp());

  const std::vector<unwindstack::FrameData>& callstack = unwinder_.Unwind(
      event->Registers(), event->StackDump(), event->StackSize());
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessUprobesCallstack(event->TID(), callstack);

  // TODO: Callstacks at the beginning and/or end of a dynamically-instrumented
  //  function could alter the statistics of time-based callstack sampling.
  //  Consider not/conditionally adding these callstacks to the trace.
  if (!full_callstack.empty() && listener_ != nullptr) {
    Callstack returned_callstack{
        event->TID(), CallstackFramesFromLibunwindstackFrames(full_callstack),
        event->Timestamp()};
    listener_->OnCallstack(returned_callstack);
  }
}

void UprobesUnwindingVisitor::visit(UretprobesPerfEvent* event) {
  std::optional<FunctionCall> function_call =
      function_call_manager_.ProcessUretprobes(event->TID(),
                                               event->Timestamp());
  if (function_call.has_value() && listener_ != nullptr) {
    listener_->OnFunctionCall(function_call.value());
  }

  callstack_manager_.ProcessUretprobes(event->TID());
}

void UprobesUnwindingVisitor::visit(MapsPerfEvent* event) {
  unwinder_.SetMaps(event->Maps());
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
