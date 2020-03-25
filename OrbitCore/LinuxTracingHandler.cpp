#include "LinuxTracingHandler.h"

#include <functional>
#include <optional>

#include "Callstack.h"
#include "ContextSwitch.h"
#include "OrbitModule.h"
#include "Params.h"
#include "Path.h"
#include "Pdb.h"
#include "TcpServer.h"
#include "llvm/Demangle/Demangle.h"

void LinuxTracingHandler::Start() {
  pid_t pid = target_process_->GetID();

  double sampling_frequency = DEFAULT_SAMPLING_FREQUENCY;

  std::vector<LinuxTracing::Function> selected_functions;
  selected_functions.reserve(selected_function_map_->size());
  for (const auto& function : *selected_function_map_) {
    selected_functions.emplace_back(
        function.second->GetPdb()->GetLoadedModuleName(),
        function.second->Offset(), function.second->GetVirtualAddress());
  }

  tracer_ = std::make_unique<LinuxTracing::Tracer>(pid, sampling_frequency,
                                                   selected_functions);

  tracer_->SetListener(this);

  tracer_->SetTraceContextSwitches(GParams.m_TrackContextSwitches);
  tracer_->SetTraceCallstacks(true);
  tracer_->SetTraceInstrumentedFunctions(true);

  tracer_->Start();
}

void LinuxTracingHandler::Stop() {
  tracer_->Stop();
  tracer_.reset();
}

void LinuxTracingHandler::OnTid(pid_t tid) {
  // TODO: This doesn't seem to be of any use at the moment: it has the effect
  //  of adding the tid to Process::m_ThreadIds in OrbitProcess.h, but that
  //  field is never actually used. Investigate whether m_ThreadIds should be
  //  used or if this call should be removed.
  target_process_->AddThreadId(tid);
}

void LinuxTracingHandler::ProcessCallstackEvent(LinuxCallstackEvent&& event) {
  CallStack cs = event.m_CS;
  if (sampling_profiler_->HasCallStack(cs.Hash())) {
    CallstackEvent hashed_callstack;
    hashed_callstack.m_Id = cs.m_Hash;
    hashed_callstack.m_TID = cs.m_ThreadId;
    hashed_callstack.m_Time = event.m_time;

    session_->RecordHashedCallstack(std::move(hashed_callstack));
  } else {
    session_->RecordCallstack(std::move(event));
  }

  // TODO: Is this needed for the case when the call stack already cached?
  sampling_profiler_->AddCallStack(cs);
}

void LinuxTracingHandler::OnContextSwitchIn(
    const LinuxTracing::ContextSwitchIn& context_switch_in) {
  ++(*num_context_switches_);

  ContextSwitch context_switch(ContextSwitch::In);
  context_switch.m_ThreadId = context_switch_in.GetTid();
  context_switch.m_Time = context_switch_in.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_in.GetCore();
  context_switch.m_ProcessorNumber = context_switch_in.GetCore();

  session_->RecordContextSwitch(std::move(context_switch));
}

void LinuxTracingHandler::OnContextSwitchOut(
    const LinuxTracing::ContextSwitchOut& context_switch_out) {
  ++(*num_context_switches_);

  ContextSwitch context_switch(ContextSwitch::Out);
  context_switch.m_ThreadId = context_switch_out.GetTid();
  context_switch.m_Time = context_switch_out.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_out.GetCore();
  context_switch.m_ProcessorNumber = context_switch_out.GetCore();

  session_->RecordContextSwitch(std::move(context_switch));
}

void LinuxTracingHandler::OnCallstack(
    const LinuxTracing::Callstack& callstack) {
  CallStack cs;
  cs.m_ThreadId = callstack.GetTid();

  for (const auto& frame : callstack.GetFrames()) {
    uint64_t address = frame.GetPc();

    if (!frame.GetFunctionName().empty() &&
        !target_process_->HasSymbol(address)) {
      std::string symbol_name =
          absl::StrFormat("%s+%#x", llvm::demangle(frame.GetFunctionName()),
                          frame.GetFunctionOffset());
      std::shared_ptr<LinuxSymbol> symbol = std::make_shared<LinuxSymbol>();
      symbol->m_Module = frame.GetMapName();
      symbol->m_Name = symbol_name;
      symbol->m_Address = address;

      // TODO: Move this out of here...
      std::string message_data = SerializeObjectBinary(*symbol);
      GTcpServer->Send(Msg_RemoteSymbol, message_data.c_str(),
                       message_data.size());

      target_process_->AddSymbol(address, symbol);
    }

    cs.m_Data.push_back(address);
  }

  cs.m_Depth = cs.m_Data.size();

  ProcessCallstackEvent({"", callstack.GetTimestampNs(), 1, cs});
}

void LinuxTracingHandler::OnFunctionCall(
    const LinuxTracing::FunctionCall& function_call) {
  Timer timer;
  timer.m_TID = function_call.GetTid();
  timer.m_Start = function_call.GetBeginTimestampNs();
  timer.m_End = function_call.GetEndTimestampNs();
  timer.m_Depth = static_cast<uint8_t>(function_call.GetDepth());
  timer.m_FunctionAddress = function_call.GetVirtualAddress();

  session_->RecordTimer(std::move(timer));
}

pid_t LinuxTracingHandler::TimelineToThreadId(std::string timeline) {
  auto it = timeline_to_thread_id_.find(timeline);
  if (it != timeline_to_thread_id_.end()) {
    return it->second;
  }
  pid_t new_id = current_timeline_thread_id;
  current_timeline_thread_id++;

  timeline_to_thread_id_.emplace(timeline, new_id);
  return new_id;
}

void LinuxTracingHandler::OnGpuJob(
    const LinuxTracing::GpuJob& gpu_job) {
  Timer timer_user_to_sched;
  timer_user_to_sched.m_TID = TimelineToThreadId(gpu_job.GetTimeline());
  timer_user_to_sched.m_Start = gpu_job.GetAmdgpuCsIoctlTimeNs();
  timer_user_to_sched.m_End = gpu_job.GetAmdgpuSchedRunJobTimeNs();
  timer_user_to_sched.m_Depth = gpu_job.GetDepth();

  /*
  const std::string sw_queue("sw queue");
  uint64_t hash = StringHash(sw_queue);
  core_app_->AddKeyAndString(hash, sw_queue);
  timer_user_to_sched.m_UserData[0] = hash;

  uint64_t timeline_hash = StringHash(gpu_job.GetTimeline());
  core_app_->AddKeyAndString(timeline_hash, gpu_job.GetTimeline());
  timer_user_to_sched.m_UserData[1] = timeline_hash;
  */

  timer_user_to_sched.m_Type = Timer::GPU_ACTIVITY;
  session_->RecordTimer(std::move(timer_user_to_sched));

  Timer timer_sched_to_start;
  timer_sched_to_start.m_TID = TimelineToThreadId(gpu_job.GetTimeline());
  timer_sched_to_start.m_Start = gpu_job.GetAmdgpuSchedRunJobTimeNs();
  timer_sched_to_start.m_End = gpu_job.GetGpuHardwareStartTimeNs();
  timer_sched_to_start.m_Depth = gpu_job.GetDepth();

  /*
  const std::string hw_queue("hw queue");
  hash = StringHash(hw_queue);
  core_app_->AddKeyAndString(hash, hw_queue);

  timer_sched_to_start.m_UserData[0] = hash;
  timer_sched_to_start.m_UserData[1] = timeline_hash;
  */

  timer_sched_to_start.m_Type = Timer::GPU_ACTIVITY;
  session_->RecordTimer(std::move(timer_sched_to_start));

  Timer timer_start_to_finish;
  timer_start_to_finish.m_TID = TimelineToThreadId(gpu_job.GetTimeline());
  timer_start_to_finish.m_Start = gpu_job.GetGpuHardwareStartTimeNs();
  timer_start_to_finish.m_End = gpu_job.GetDmaFenceSignaledTimeNs();
  timer_start_to_finish.m_Depth = gpu_job.GetDepth();

  /*
  const std::string hw_execution("hw execution");
  hash = StringHash(hw_execution);
  core_app_->AddKeyAndString(hash, hw_execution);
  timer_start_to_finish.m_UserData[0] = hash;
  timer_start_to_finish.m_UserData[1] = timeline_hash;
  */

  timer_start_to_finish.m_Type = Timer::GPU_ACTIVITY;
  session_->RecordTimer(std::move(timer_start_to_finish));
}
