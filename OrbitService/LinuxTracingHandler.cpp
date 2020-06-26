// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingHandler.h"

#include "Callstack.h"
#include "absl/flags/flag.h"
#include "llvm/Demangle/Demangle.h"

void LinuxTracingHandler::Start(CaptureOptions capture_options) {
  addresses_seen_.clear();
  callstack_hashes_seen_.clear();
  string_keys_seen_.clear();

  tracer_ = std::make_unique<LinuxTracing::Tracer>(std::move(capture_options));
  tracer_->SetListener(this);
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

void LinuxTracingHandler::OnSchedulingSlice(SchedulingSlice scheduling_slice) {
  Timer timer;
  timer.m_Start = scheduling_slice.in_timestamp_ns();
  timer.m_End = scheduling_slice.out_timestamp_ns();
  timer.m_PID = scheduling_slice.pid();
  timer.m_TID = scheduling_slice.tid();
  timer.m_Processor = static_cast<int8_t>(scheduling_slice.core());
  timer.m_Depth = timer.m_Processor;
  timer.SetType(Timer::CORE_ACTIVITY);

  tracing_buffer_->RecordTimer(std::move(timer));
}

void LinuxTracingHandler::OnCallstackSample(CallstackSample callstack_sample) {
  CallStack cs;
  cs.m_ThreadId = callstack_sample.tid();
  CHECK(callstack_sample.callstack_or_key_case() ==
        CallstackSample::kCallstack);
  for (const auto& frame : callstack_sample.callstack().pcs()) {
    cs.m_Data.push_back(frame);
  }
  cs.m_Depth = cs.m_Data.size();
  CallstackID cs_hash = cs.Hash();

  {
    absl::MutexLock lock{&callstack_hashes_seen_mutex_};
    if (callstack_hashes_seen_.contains(cs_hash)) {
      CallstackEvent hashed_cs_event{callstack_sample.timestamp_ns(), cs.m_Hash,
                                     cs.m_ThreadId};
      tracing_buffer_->RecordHashedCallstack(std::move(hashed_cs_event));
    } else {
      LinuxCallstackEvent cs_event{callstack_sample.timestamp_ns(), cs};
      tracing_buffer_->RecordCallstack(std::move(cs_event));
      callstack_hashes_seen_.insert(cs_hash);
    }
  }
}

void LinuxTracingHandler::OnFunctionCall(FunctionCall function_call) {
  Timer timer;
  timer.m_TID = function_call.tid();
  timer.m_Start = function_call.begin_timestamp_ns();
  timer.m_End = function_call.end_timestamp_ns();
  timer.m_Depth = static_cast<uint8_t>(function_call.depth());
  timer.m_FunctionAddress = function_call.absolute_address();
  timer.m_UserData[0] = function_call.return_value();

  tracing_buffer_->RecordTimer(std::move(timer));
}

uint64_t LinuxTracingHandler::ProcessStringAndGetKey(
    const std::string& string) {
  uint64_t key = StringHash(string);
  if (!string_keys_seen_.contains(key)) {
    string_keys_seen_.emplace(key);
    tracing_buffer_->RecordKeyAndString(key, string);
  }
  return key;
}

void LinuxTracingHandler::OnGpuJob(GpuJob gpu_job) {
  Timer timer_user_to_sched;
  timer_user_to_sched.m_TID = gpu_job.tid();
  timer_user_to_sched.m_Start = gpu_job.amdgpu_cs_ioctl_time_ns();
  timer_user_to_sched.m_End = gpu_job.amdgpu_sched_run_job_time_ns();
  timer_user_to_sched.m_Depth = gpu_job.depth();

  constexpr const char* sw_queue = "sw queue";
  uint64_t sw_queue_key = ProcessStringAndGetKey(sw_queue);
  timer_user_to_sched.m_UserData[0] = sw_queue_key;

  CHECK(gpu_job.timeline_or_key_case() == GpuJob::kTimeline);
  uint64_t timeline_key = ProcessStringAndGetKey(gpu_job.timeline());
  timer_user_to_sched.m_UserData[1] = timeline_key;

  timer_user_to_sched.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_user_to_sched));

  Timer timer_sched_to_start;
  timer_sched_to_start.m_TID = gpu_job.tid();
  timer_sched_to_start.m_Start = gpu_job.amdgpu_sched_run_job_time_ns();
  timer_sched_to_start.m_End = gpu_job.gpu_hardware_start_time_ns();
  timer_sched_to_start.m_Depth = gpu_job.depth();

  constexpr const char* hw_queue = "hw queue";
  uint64_t hw_queue_key = ProcessStringAndGetKey(hw_queue);

  timer_sched_to_start.m_UserData[0] = hw_queue_key;
  timer_sched_to_start.m_UserData[1] = timeline_key;

  timer_sched_to_start.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_sched_to_start));

  Timer timer_start_to_finish;
  timer_start_to_finish.m_TID = gpu_job.tid();
  timer_start_to_finish.m_Start = gpu_job.gpu_hardware_start_time_ns();
  timer_start_to_finish.m_End = gpu_job.dma_fence_signaled_time_ns();
  timer_start_to_finish.m_Depth = gpu_job.depth();

  constexpr const char* hw_execution = "hw execution";
  uint64_t hw_execution_key = ProcessStringAndGetKey(hw_execution);

  timer_start_to_finish.m_UserData[0] = hw_execution_key;
  timer_start_to_finish.m_UserData[1] = timeline_key;

  timer_start_to_finish.m_Type = Timer::GPU_ACTIVITY;
  tracing_buffer_->RecordTimer(std::move(timer_start_to_finish));
}

void LinuxTracingHandler::OnThreadName(pid_t tid, const std::string& name) {
  tracing_buffer_->RecordThreadName(tid, name);
}

void LinuxTracingHandler::OnAddressInfo(AddressInfo address_info) {
  absl::MutexLock lock{&addresses_seen_mutex_};
  // This provides the client with some symbol information that Libunwindstack
  // was able to extract from the binary while unwinding. The client can use it
  // to assign function/map names and to aggregate IPs by function for addresses
  // belonging to modules for which symbols haven't been loaded.
  if (!addresses_seen_.contains(address_info.absolute_address())) {
    LinuxAddressInfo linux_address_info{
        address_info.absolute_address(), address_info.map_name(),
        llvm::demangle(address_info.function_name()),
        address_info.offset_in_function()};
    tracing_buffer_->RecordAddressInfo(std::move(linux_address_info));
    addresses_seen_.insert(address_info.absolute_address());
  }
}
