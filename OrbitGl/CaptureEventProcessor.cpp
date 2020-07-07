#include "CaptureEventProcessor.h"

void CaptureEventProcessor::ProcessEvent(const CaptureEvent& event) {
  switch (event.event_case()) {
    case CaptureEvent::kSchedulingSlice:
      ProcessSchedulingSlice(event.scheduling_slice());
      break;
    case CaptureEvent::kInternedCallstack:
      ProcessInternedCallstack(event.interned_callstack());
      break;
    case CaptureEvent::kCallstackSample:
      ProcessCallstackSample(event.callstack_sample());
      break;
    case CaptureEvent::kFunctionCall:
      ProcessFunctionCall(event.function_call());
      break;
    case CaptureEvent::kInternedString:
      ProcessInternedString(event.interned_string());
      break;
    case CaptureEvent::kGpuJob:
      ProcessGpuJob(event.gpu_job());
      break;
    case CaptureEvent::kThreadName:
      ProcessThreadName(event.thread_name());
      break;
    case CaptureEvent::kAddressInfo:
      ProcessAddressInfo(event.address_info());
      break;
    case CaptureEvent::EVENT_NOT_SET:
      ERROR("CaptureEvent::EVENT_NOT_SET read from Capture's gRPC stream");
      break;
  }
}

void CaptureEventProcessor::ProcessSchedulingSlice(
    const SchedulingSlice& scheduling_slice) {
  Timer timer;
  timer.m_Start = scheduling_slice.in_timestamp_ns();
  timer.m_End = scheduling_slice.out_timestamp_ns();
  timer.m_PID = scheduling_slice.pid();
  timer.m_TID = scheduling_slice.tid();
  timer.m_Processor = static_cast<int8_t>(scheduling_slice.core());
  timer.m_Depth = timer.m_Processor;
  timer.SetType(Timer::CORE_ACTIVITY);

  capture_listener_->OnTimer(timer);
}

void CaptureEventProcessor::ProcessInternedCallstack(
    InternedCallstack interned_callstack) {
  if (callstack_intern_pool.contains(interned_callstack.key())) {
    ERROR("Overwriting InternedCallstack with key %llu",
          interned_callstack.key());
  }
  callstack_intern_pool.emplace(
      interned_callstack.key(),
      std::move(*interned_callstack.mutable_intern()));
}

void CaptureEventProcessor::ProcessCallstackSample(
    const CallstackSample& callstack_sample) {
  Callstack callstack;
  if (callstack_sample.callstack_or_key_case() ==
      CallstackSample::kCallstackKey) {
    callstack = callstack_intern_pool[callstack_sample.callstack_key()];
  } else {
    callstack = callstack_sample.callstack();
  }

  uint64_t hash = GetCallstackHashAndSendToListenerIfNecessary(callstack);
  CallstackEvent callstack_event{callstack_sample.timestamp_ns(), hash,
                                 callstack_sample.tid()};
  capture_listener_->OnCallstackEvent(std::move(callstack_event));
}

void CaptureEventProcessor::ProcessFunctionCall(
    const FunctionCall& function_call) {
  Timer timer;
  timer.m_TID = function_call.tid();
  timer.m_Start = function_call.begin_timestamp_ns();
  timer.m_End = function_call.end_timestamp_ns();
  timer.m_Depth = static_cast<uint8_t>(function_call.depth());
  timer.m_FunctionAddress = function_call.absolute_address();
  timer.m_UserData[0] = function_call.return_value();

  capture_listener_->OnTimer(timer);
}

void CaptureEventProcessor::ProcessInternedString(
    InternedString interned_string) {
  if (string_intern_pool.contains(interned_string.key())) {
    ERROR("Overwriting InternedString with key %llu", interned_string.key());
  }
  string_intern_pool.emplace(interned_string.key(),
                             std::move(*interned_string.mutable_intern()));
}

void CaptureEventProcessor::ProcessGpuJob(const GpuJob& gpu_job) {
  Timer timer_user_to_sched;
  timer_user_to_sched.m_TID = gpu_job.tid();
  timer_user_to_sched.m_Start = gpu_job.amdgpu_cs_ioctl_time_ns();
  timer_user_to_sched.m_End = gpu_job.amdgpu_sched_run_job_time_ns();
  timer_user_to_sched.m_Depth = gpu_job.depth();

  std::string timeline;
  if (gpu_job.timeline_or_key_case() == GpuJob::kTimelineKey) {
    timeline = string_intern_pool[gpu_job.timeline_key()];
  } else {
    timeline = gpu_job.timeline();
  }
  uint64_t timeline_hash = GetStringHashAndSendToListenerIfNecessary(timeline);

  constexpr const char* sw_queue = "sw queue";
  uint64_t sw_queue_key = GetStringHashAndSendToListenerIfNecessary(sw_queue);
  timer_user_to_sched.m_UserData[0] = sw_queue_key;
  timer_user_to_sched.m_UserData[1] = timeline_hash;

  timer_user_to_sched.m_Type = Timer::GPU_ACTIVITY;
  capture_listener_->OnTimer(std::move(timer_user_to_sched));

  Timer timer_sched_to_start;
  timer_sched_to_start.m_TID = gpu_job.tid();
  timer_sched_to_start.m_Start = gpu_job.amdgpu_sched_run_job_time_ns();
  timer_sched_to_start.m_End = gpu_job.gpu_hardware_start_time_ns();
  timer_sched_to_start.m_Depth = gpu_job.depth();

  constexpr const char* hw_queue = "hw queue";
  uint64_t hw_queue_key = GetStringHashAndSendToListenerIfNecessary(hw_queue);

  timer_sched_to_start.m_UserData[0] = hw_queue_key;
  timer_sched_to_start.m_UserData[1] = timeline_hash;

  timer_sched_to_start.m_Type = Timer::GPU_ACTIVITY;
  capture_listener_->OnTimer(std::move(timer_sched_to_start));

  Timer timer_start_to_finish;
  timer_start_to_finish.m_TID = gpu_job.tid();
  timer_start_to_finish.m_Start = gpu_job.gpu_hardware_start_time_ns();
  timer_start_to_finish.m_End = gpu_job.dma_fence_signaled_time_ns();
  timer_start_to_finish.m_Depth = gpu_job.depth();

  constexpr const char* hw_execution = "hw execution";
  uint64_t hw_execution_key =
      GetStringHashAndSendToListenerIfNecessary(hw_execution);

  timer_start_to_finish.m_UserData[0] = hw_execution_key;
  timer_start_to_finish.m_UserData[1] = timeline_hash;

  timer_start_to_finish.m_Type = Timer::GPU_ACTIVITY;
  capture_listener_->OnTimer(std::move(timer_start_to_finish));
}

void CaptureEventProcessor::ProcessThreadName(const ThreadName& thread_name) {
  capture_listener_->OnThreadName(thread_name.tid(), thread_name.name());
}

void CaptureEventProcessor::ProcessAddressInfo(
    const AddressInfo& address_info) {
  std::string function_name;
  if (address_info.function_name_or_key_case() ==
      AddressInfo::kFunctionNameKey) {
    function_name = string_intern_pool[address_info.function_name_key()];
  } else {
    function_name = address_info.function_name();
  }

  std::string map_name;
  if (address_info.map_name_or_key_case() == AddressInfo::kMapNameKey) {
    map_name = string_intern_pool[address_info.map_name_key()];
  } else {
    map_name = address_info.map_name();
  }

  LinuxAddressInfo linux_address_info{address_info.absolute_address(), map_name,
                                      function_name,
                                      address_info.offset_in_function()};
  capture_listener_->OnAddressInfo(linux_address_info);
}

uint64_t CaptureEventProcessor::GetCallstackHashAndSendToListenerIfNecessary(
    const Callstack& callstack) {
  CallStack cs;
  for (uint64_t pc : callstack.pcs()) {
    cs.m_Data.push_back(pc);
  }
  cs.m_Depth = cs.m_Data.size();
  // TODO: Compute the hash without creating the CallStack if not necessary.
  uint64_t hash = cs.Hash();

  if (!callstack_hashes_seen_.contains(hash)) {
    callstack_hashes_seen_.emplace(hash);
    capture_listener_->OnCallstack(cs);
  }
  return hash;
}

uint64_t CaptureEventProcessor::GetStringHashAndSendToListenerIfNecessary(
    const std::string& str) {
  uint64_t hash = StringHash(str);
  if (!string_hashes_seen_.contains(hash)) {
    string_hashes_seen_.emplace(hash);
    capture_listener_->OnKeyAndString(hash, str);
  }
  return hash;
}