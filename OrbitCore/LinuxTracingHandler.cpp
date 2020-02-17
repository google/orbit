#include "LinuxTracingHandler.h"

#include <functional>
#include <optional>

#include "ContextSwitch.h"
#include "OrbitModule.h"
#include "Params.h"
#include "Path.h"
#include "Pdb.h"

void LinuxTracingHandler::Start() {
  pid_t pid = target_process_->GetID();

  double sampling_frequency = DEFAULT_SAMPLING_FREQUENCY;

  std::vector<LinuxTracing::Function> selected_functions;
  selected_functions.reserve(selected_function_map_->size());
  for (const auto& function : *selected_function_map_) {
    selected_functions.emplace_back(
        function.second->GetPdb()->GetLoadedModuleName(),
        function.second->Address(), function.second->GetVirtualAddress());
  }

  tracer_ = std::make_unique<LinuxTracing::Tracer>(pid, sampling_frequency,
                                                   selected_functions);

  tracer_->SetListener(this);

  tracer_->SetTraceContextSwitches(GParams.m_TrackContextSwitches);
  tracer_->SetTraceCallstacks(!GParams.m_SampleWithPerf);
  tracer_->SetTraceInstrumentedFunctions(true);

  tracer_->Start();
}

void LinuxTracingHandler::Stop() {
  tracer_->Stop();
  tracer_.reset();
}

void LinuxTracingHandler::OnTid(pid_t tid) {
  target_process_->AddThreadId(tid);
}

void LinuxTracingHandler::OnContextSwitchIn(
    const LinuxTracing::ContextSwitchIn& context_switch_in) {
  ++(*num_context_switches_);

  ContextSwitch context_switch(ContextSwitch::In);
  context_switch.m_ThreadId = context_switch_in.GetTid();
  context_switch.m_Time = context_switch_in.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_in.GetCore();
  context_switch.m_ProcessorNumber = context_switch_in.GetCore();
  core_app_->ProcessContextSwitch(context_switch);
}

void LinuxTracingHandler::OnContextSwitchOut(
    const LinuxTracing::ContextSwitchOut& context_switch_out) {
  ++(*num_context_switches_);

  ContextSwitch context_switch(ContextSwitch::Out);
  context_switch.m_ThreadId = context_switch_out.GetTid();
  context_switch.m_Time = context_switch_out.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_out.GetCore();
  context_switch.m_ProcessorNumber = context_switch_out.GetCore();
  core_app_->ProcessContextSwitch(context_switch);
}

void LinuxTracingHandler::OnCallstack(
    const LinuxTracing::Callstack& callstack) {
  CallStack cs;
  cs.m_ThreadId = callstack.GetTid();

  for (const auto& frame : callstack.GetFrames()) {
    // TODO: Avoid repeating symbol resolution by caching already seen PCs.
    //  If this validation of PC is even necessary, that is.
    uint64_t address = frame.GetPc();

    std::string module_name = ToLower(Path::GetFileName(frame.GetMapName()));
    std::shared_ptr<Module> module =
        target_process_->GetModuleFromName(module_name);
    if (module) {
      address = module->ValidateAddress(address);
    }

    if (!frame.GetFunctionName().empty() &&
        !target_process_->HasSymbol(address)) {
      std::string symbol_name = absl::StrFormat(
          "%s+%#x", LinuxUtils::Demangle(frame.GetFunctionName().c_str()),
          frame.GetFunctionOffset());
      core_app_->AddSymbol(address, frame.GetMapName(), symbol_name);
    }

    cs.m_Data.push_back(address);
  }

  cs.m_Depth = cs.m_Data.size();

  LinuxCallstackEvent callstack_event{"", callstack.GetTimestampNs(), 1, cs};
  core_app_->ProcessSamplingCallStack(callstack_event);
}

void LinuxTracingHandler::OnFunctionBegin(
    const LinuxTracing::FunctionBegin& function_begin) {
  auto& tid_timer_stack = tid_timer_stacks_[function_begin.GetTid()];
  Timer timer;
  timer.m_TID = function_begin.GetTid();
  timer.m_Start = function_begin.GetTimestampNs();
  timer.m_Depth = static_cast<uint8_t>(tid_timer_stack.size());
  timer.m_FunctionAddress = function_begin.GetVirtualAddress();
  tid_timer_stack.push(timer);
}

void LinuxTracingHandler::OnFunctionEnd(
    const LinuxTracing::FunctionEnd& function_end) {
  if (tid_timer_stacks_.count(function_end.GetTid()) > 0) {
    auto& timer_stack = tid_timer_stacks_.at(function_end.GetTid());

    // This is because we erase the stack for this thread as soon as it becomes
    // empty.
    assert(!timer_stack.empty());

    if (!timer_stack.empty()) {
      Timer timer = timer_stack.top();
      timer_stack.pop();
      if (timer_stack.empty()) {
        tid_timer_stacks_.erase(function_end.GetTid());
      }

      timer.m_End = function_end.GetTimestampNs();
      core_app_->ProcessTimer(timer, std::to_string(timer.m_FunctionAddress));
    }
  }
}
