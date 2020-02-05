#include "LinuxUprobesUnwindingVisitor.h"

#include "Callstack.h"
#include "Capture.h"
#include "CoreApp.h"
#include "LinuxUtils.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Path.h"

void UprobesTimerManager::ProcessUprobes(pid_t tid, uint64_t begin_timestamp,
                                         uint64_t function_address) {
  std::vector<Timer>& tid_timer_stack = tid_timer_stacks_[tid];
  Timer timer;
  timer.m_TID = tid;
  timer.m_Start = begin_timestamp;
  timer.m_Depth = static_cast<uint8_t>(tid_timer_stack.size());
  timer.m_FunctionAddress = function_address;
  tid_timer_stack.push_back(timer);
}

bool UprobesTimerManager::ProcessUretprobes(pid_t tid, uint64_t end_timestamp,
                                            Timer* timer) {
  std::vector<Timer>& tid_timer_stack = tid_timer_stacks_[tid];

  if (!tid_timer_stack.empty()) {
    *timer = tid_timer_stack.back();
    tid_timer_stack.pop_back();
    timer->m_End = end_timestamp;
    if (tid_timer_stack.empty()) {
      tid_timer_stacks_.erase(tid);
    }
    return true;
  } else {
    return false;
  }
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

std::vector<unwindstack::FrameData>
UprobesCallstackManager::ProcessUretprobesCallstack(
    pid_t tid, const std::vector<unwindstack::FrameData>& callstack) {
  std::vector<std::vector<unwindstack::FrameData>>& previous_callstacks =
      tid_uprobes_callstacks_stacks_[tid];
  if (!previous_callstacks.empty()) {
    previous_callstacks.pop_back();
  }

  const std::vector<unwindstack::FrameData>& full_callstack =
      JoinCallstackWithPreviousUprobesCallstacks(callstack,
                                                 previous_callstacks);

  if (previous_callstacks.empty()) {
    tid_uprobes_callstacks_stacks_.erase(tid);
  }

  return full_callstack;
}

void LinuxUprobesUnwindingVisitor::visit(LinuxStackSampleEvent* event) {
  const std::vector<unwindstack::FrameData>& callstack = unwinder_.Unwind(
      event->Registers(), event->StackDump(), event->StackSize());
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessSampledCallstack(event->TID(), callstack);
  if (!full_callstack.empty()) {
    HandleCallstack(event->TID(), event->Timestamp(), full_callstack);
  }
}

void LinuxUprobesUnwindingVisitor::visit(LinuxUprobeEventWithStack* event) {
  timer_manager_.ProcessUprobes(event->TID(), event->Timestamp(),
                                event->GetFunction()->GetVirtualAddress());

  const std::vector<unwindstack::FrameData>& callstack = unwinder_.Unwind(
      event->Registers(), event->StackDump(), event->StackSize());
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessUprobesCallstack(event->TID(), callstack);

  // TODO: Callstacks at the beginning and/or end of a dynamically-instrumented
  //  function could alter the statistics of time-based callstack sampling.
  //  Consider not/conditionally adding these callstacks to the trace.
  if (!full_callstack.empty()) {
    HandleCallstack(event->TID(), event->Timestamp(), full_callstack);
  }
}

void LinuxUprobesUnwindingVisitor::visit(LinuxUretprobeEventWithStack* event) {
  Timer timer;
  if (timer_manager_.ProcessUretprobes(event->TID(), event->Timestamp(),
                                       &timer)) {
    HandleTimer(timer);
  }

  const std::vector<unwindstack::FrameData>& callstack = unwinder_.Unwind(
      event->Registers(), event->StackDump(), event->StackSize());
  const std::vector<unwindstack::FrameData>& full_callstack =
      callstack_manager_.ProcessUretprobesCallstack(event->TID(), callstack);
  // Remove this if we do not want a callstack at the return of an instrumented
  // function.
  if (!full_callstack.empty()) {
    HandleCallstack(event->TID(), event->Timestamp(), full_callstack);
  }
}

void LinuxUprobesUnwindingVisitor::visit(LinuxMapsEvent* event) {
  unwinder_.SetMaps(event->Maps());
}

void LinuxUprobesUnwindingVisitor::HandleTimer(const Timer& timer) {
  GCoreApp->ProcessTimer(timer, std::to_string(timer.m_FunctionAddress));
}

void LinuxUprobesUnwindingVisitor::HandleCallstack(
    pid_t tid, uint64_t timestamp,
    const std::vector<unwindstack::FrameData>& frames) {
  CallStack cs;
  cs.m_ThreadId = tid;
  for (const auto& frame : frames) {
    // TODO: Avoid repeating symbol resolution by caching already seen PCs.
    std::string moduleName = ToLower(Path::GetFileName(frame.map_name));
    std::shared_ptr<Module> moduleFromName =
        Capture::GTargetProcess->GetModuleFromName(moduleName);

    uint64_t address = frame.pc;
    if (moduleFromName) {
      uint64_t new_address = moduleFromName->ValidateAddress(address);
      address = new_address;
    }
    cs.m_Data.push_back(address);

    if (!frame.function_name.empty() &&
        !Capture::GTargetProcess->HasSymbol(address)) {
      std::stringstream ss;
      ss << LinuxUtils::Demangle(frame.function_name.c_str()) << "+0x"
         << std::hex << frame.function_offset;
      GCoreApp->AddSymbol(address, frame.map_name, ss.str());
    }
  }
  cs.m_Depth = cs.m_Data.size();

  LinuxCallstackEvent callstack_event{"", timestamp, 1, cs};
  GCoreApp->ProcessSamplingCallStack(callstack_event);
}
