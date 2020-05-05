#include "LinuxTracingHandler.h"

#include <functional>

#include "Callstack.h"
#include "ContextSwitch.h"
#include "OrbitLinuxTracing/Events.h"
#include "OrbitLinuxTracing/TracingOptions.h"
#include "OrbitModule.h"
#include "Params.h"
#include "absl/flags/flag.h"
#include "llvm/Demangle/Demangle.h"

// TODO: Remove this flag once we enable specifying the sampling frequency or
//  period in the client.
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");

// TODO: This is a temporary feature flag. Remove this once we enable this
//  globally.
ABSL_FLAG(bool, trace_gpu_driver, false,
          "Enables tracing of GPU driver tracepoint events");

void LinuxTracingHandler::Start(
    pid_t pid, const std::map<uint64_t, Function*>& selected_function_map) {
  addresses_seen_.clear();
  callstack_hashes_seen_.clear();
  string_manager_.Clear();

  double sampling_rate = absl::GetFlag(FLAGS_sampling_rate);

  std::vector<LinuxTracing::Function> selected_functions;
  selected_functions.reserve(selected_function_map.size());
  for (const auto& pair : selected_function_map) {
    const auto& function = pair.second;
    selected_functions.emplace_back(function->GetLoadedModuleName(),
                                    function->Offset(),
                                    function->GetVirtualAddress());
  }

  tracer_ = std::make_unique<LinuxTracing::Tracer>(pid, sampling_rate,
                                                   selected_functions);

  tracer_->SetListener(this);

  tracer_->SetTraceContextSwitches(GParams.m_TrackContextSwitches);
  tracer_->SetSamplingMethod(LinuxTracing::kDwarf);
  tracer_->SetTraceInstrumentedFunctions(true);
  tracer_->SetTraceGpuDriver(absl::GetFlag(FLAGS_trace_gpu_driver));

  tracer_->Start();
}

bool LinuxTracingHandler::IsStarted() {
  return tracer_ != nullptr && tracer_->IsTracing();
}

void LinuxTracingHandler::Stop() {
  if (tracer_ != nullptr) {
    tracer_->Stop();
  }
  tracer_.reset();
}

void LinuxTracingHandler::OnTid(pid_t /*tid*/) {
  // Do nothing.
}

void LinuxTracingHandler::OnContextSwitchIn(
    const LinuxTracing::ContextSwitchIn& context_switch_in) {
  ContextSwitch context_switch(ContextSwitch::In);
  context_switch.m_ProcessId = context_switch_in.GetPid();
  context_switch.m_ThreadId = context_switch_in.GetTid();
  context_switch.m_Time = context_switch_in.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_in.GetCore();
  context_switch.m_ProcessorNumber = context_switch_in.GetCore();

  tracing_buffer_->RecordContextSwitch(std::move(context_switch));
}

void LinuxTracingHandler::OnContextSwitchOut(
    const LinuxTracing::ContextSwitchOut& context_switch_out) {
  ContextSwitch context_switch(ContextSwitch::Out);
  context_switch.m_ProcessId = context_switch_out.GetPid();
  context_switch.m_ThreadId = context_switch_out.GetTid();
  context_switch.m_Time = context_switch_out.GetTimestampNs();
  context_switch.m_ProcessorIndex = context_switch_out.GetCore();
  context_switch.m_ProcessorNumber = context_switch_out.GetCore();

  tracing_buffer_->RecordContextSwitch(std::move(context_switch));
}

void LinuxTracingHandler::OnCallstack(
    const LinuxTracing::Callstack& callstack) {
  CallStack cs;
  cs.m_ThreadId = callstack.GetTid();

  for (const auto& frame : callstack.GetFrames()) {
    uint64_t address = frame.GetPc();
    cs.m_Data.push_back(address);

    // TODO(kuebler): This is mainly for clustering IPs to their functions.
    //  We should enable this also as a post-processing step.
    if (frame.GetFunctionOffset() !=
        LinuxTracing::CallstackFrame::kUnknownFunctionOffset) {
      absl::MutexLock lock{&addresses_seen_mutex_};
      if (!addresses_seen_.contains(address)) {
        LinuxAddressInfo address_info{address, frame.GetMapName(),
                                      llvm::demangle(frame.GetFunctionName()),
                                      frame.GetFunctionOffset()};
        tracing_buffer_->RecordAddressInfo(std::move(address_info));
        addresses_seen_.insert(address);
      }
    }
  }

  cs.m_Depth = cs.m_Data.size();
  CallstackID cs_hash = cs.Hash();

  {
    absl::MutexLock lock{&callstack_hashes_seen_mutex_};
    if (callstack_hashes_seen_.contains(cs_hash)) {
      CallstackEvent hashed_cs_event{callstack.GetTimestampNs(), cs.m_Hash,
                                     cs.m_ThreadId};
      tracing_buffer_->RecordHashedCallstack(std::move(hashed_cs_event));
    } else {
      LinuxCallstackEvent cs_event{callstack.GetTimestampNs(), cs};
      tracing_buffer_->RecordCallstack(std::move(cs_event));
      callstack_hashes_seen_.insert(cs_hash);
    }
  }
}

void LinuxTracingHandler::OnFunctionCall(
    const LinuxTracing::FunctionCall& function_call) {
  Timer timer;
  timer.m_TID = function_call.GetTid();
  timer.m_Start = function_call.GetBeginTimestampNs();
  timer.m_End = function_call.GetEndTimestampNs();
  timer.m_Depth = static_cast<uint8_t>(function_call.GetDepth());
  timer.m_FunctionAddress = function_call.GetVirtualAddress();
  timer.m_UserData[0] = function_call.GetIntegerReturnValue();

  tracing_buffer_->RecordTimer(std::move(timer));
}

uint64_t LinuxTracingHandler::ProcessStringAndGetKey(
    const std::string& string) {
  uint64_t key = StringHash(string);
  if (string_manager_.AddIfNotPresent(key, string)) {
    tracing_buffer_->RecordKeyAndString(key, string);
  }
  return key;
}

void LinuxTracingHandler::OnGpuJob(const LinuxTracing::GpuJob& gpu_job) {
  Timer timer_user_to_sched;
  timer_user_to_sched.m_TID = gpu_job.GetTid();
  timer_user_to_sched.m_Start = gpu_job.GetAmdgpuCsIoctlTimeNs();
  timer_user_to_sched.m_End = gpu_job.GetAmdgpuSchedRunJobTimeNs();
  timer_user_to_sched.m_Depth = gpu_job.GetDepth();

  constexpr const char* sw_queue = "sw queue";
  uint64_t sw_queue_key = ProcessStringAndGetKey(sw_queue);
  timer_user_to_sched.m_UserData[0] = sw_queue_key;

  uint64_t timeline_key = ProcessStringAndGetKey(gpu_job.GetTimeline());
  timer_user_to_sched.m_UserData[1] = timeline_key;

  timer_user_to_sched.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_user_to_sched));

  Timer timer_sched_to_start;
  timer_sched_to_start.m_TID = gpu_job.GetTid();
  timer_sched_to_start.m_Start = gpu_job.GetAmdgpuSchedRunJobTimeNs();
  timer_sched_to_start.m_End = gpu_job.GetGpuHardwareStartTimeNs();
  timer_sched_to_start.m_Depth = gpu_job.GetDepth();

  constexpr const char* hw_queue = "hw queue";
  uint64_t hw_queue_key = ProcessStringAndGetKey(hw_queue);

  timer_sched_to_start.m_UserData[0] = hw_queue_key;
  timer_sched_to_start.m_UserData[1] = timeline_key;

  timer_sched_to_start.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_sched_to_start));

  Timer timer_start_to_finish;
  timer_start_to_finish.m_TID = gpu_job.GetTid();
  timer_start_to_finish.m_Start = gpu_job.GetGpuHardwareStartTimeNs();
  timer_start_to_finish.m_End = gpu_job.GetDmaFenceSignaledTimeNs();
  timer_start_to_finish.m_Depth = gpu_job.GetDepth();

  constexpr const char* hw_execution = "hw execution";
  uint64_t hw_execution_key = ProcessStringAndGetKey(hw_execution);

  timer_start_to_finish.m_UserData[0] = hw_execution_key;
  timer_start_to_finish.m_UserData[1] = timeline_key;

  timer_start_to_finish.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_start_to_finish));
}
