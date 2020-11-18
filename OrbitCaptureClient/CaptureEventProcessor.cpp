// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureEventProcessor.h"

#include "CoreUtils.h"
#include "OrbitBase/Tracing.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::ThreadStateSliceInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureEvent;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::InternedCallstack;
using orbit_grpc_protos::InternedString;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;

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
    case CaptureEvent::kIntrospectionScope:
      ProcessIntrospectionScope(event.introspection_scope());
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
    case CaptureEvent::kThreadStateSlice:
      ProcessThreadStateSlice(event.thread_state_slice());
      break;
    case CaptureEvent::kAddressInfo:
      ProcessAddressInfo(event.address_info());
      break;
    case CaptureEvent::kInternedTracepointInfo:
      ProcessInternedTracepointInfo(event.interned_tracepoint_info());
      break;
    case CaptureEvent::kTracepointEvent:
      ProcessTracepointEvent(event.tracepoint_event());
      break;
    case CaptureEvent::EVENT_NOT_SET:
      ERROR("CaptureEvent::EVENT_NOT_SET read from Capture's gRPC stream");
      break;
  }
}

void CaptureEventProcessor::ProcessSchedulingSlice(const SchedulingSlice& scheduling_slice) {
  TimerInfo timer_info;
  timer_info.set_start(scheduling_slice.in_timestamp_ns());
  timer_info.set_end(scheduling_slice.out_timestamp_ns());
  timer_info.set_process_id(scheduling_slice.pid());
  timer_info.set_thread_id(scheduling_slice.tid());
  timer_info.set_processor(static_cast<int8_t>(scheduling_slice.core()));
  timer_info.set_depth(timer_info.processor());
  timer_info.set_type(TimerInfo::kCoreActivity);

  capture_listener_->OnTimer(timer_info);
}

void CaptureEventProcessor::ProcessInternedCallstack(InternedCallstack interned_callstack) {
  if (callstack_intern_pool.contains(interned_callstack.key())) {
    ERROR("Overwriting InternedCallstack with key %llu", interned_callstack.key());
  }
  callstack_intern_pool.emplace(interned_callstack.key(),
                                std::move(*interned_callstack.mutable_intern()));
}

void CaptureEventProcessor::ProcessCallstackSample(const CallstackSample& callstack_sample) {
  Callstack callstack;
  if (callstack_sample.callstack_or_key_case() == CallstackSample::kCallstackKey) {
    callstack = callstack_intern_pool[callstack_sample.callstack_key()];
  } else {
    callstack = callstack_sample.callstack();
  }

  uint64_t hash = GetCallstackHashAndSendToListenerIfNecessary(callstack);
  CallstackEvent callstack_event;
  callstack_event.set_time(callstack_sample.timestamp_ns());
  callstack_event.set_callstack_hash(hash);
  callstack_event.set_thread_id(callstack_sample.tid());
  capture_listener_->OnCallstackEvent(std::move(callstack_event));
}

void CaptureEventProcessor::ProcessFunctionCall(const FunctionCall& function_call) {
  TimerInfo timer_info;
  timer_info.set_process_id(function_call.pid());
  timer_info.set_thread_id(function_call.tid());
  timer_info.set_start(function_call.begin_timestamp_ns());
  timer_info.set_end(function_call.end_timestamp_ns());
  timer_info.set_depth(static_cast<uint8_t>(function_call.depth()));
  timer_info.set_function_address(function_call.absolute_address());
  timer_info.set_user_data_key(function_call.return_value());
  timer_info.set_processor(-1);
  timer_info.set_type(TimerInfo::kNone);

  for (int i = 0; i < function_call.registers_size(); ++i) {
    timer_info.add_registers(function_call.registers(i));
  }

  capture_listener_->OnTimer(timer_info);
}

void CaptureEventProcessor::ProcessIntrospectionScope(
    const IntrospectionScope& introspection_scope) {
  TimerInfo timer_info;
  timer_info.set_process_id(introspection_scope.pid());
  timer_info.set_thread_id(introspection_scope.tid());
  timer_info.set_start(introspection_scope.begin_timestamp_ns());
  timer_info.set_end(introspection_scope.end_timestamp_ns());
  timer_info.set_depth(static_cast<uint8_t>(introspection_scope.depth()));
  timer_info.set_function_address(0);  // function address n/a, set to invalid value
  timer_info.set_processor(-1);        // cpu info not available, set to invalid value
  timer_info.set_type(TimerInfo::kIntrospection);
  timer_info.mutable_registers()->CopyFrom(introspection_scope.registers());
  capture_listener_->OnTimer(timer_info);
}

void CaptureEventProcessor::ProcessInternedString(InternedString interned_string) {
  if (string_intern_pool.contains(interned_string.key())) {
    ERROR("Overwriting InternedString with key %llu", interned_string.key());
  }
  string_intern_pool.emplace(interned_string.key(), std::move(*interned_string.mutable_intern()));
}

void CaptureEventProcessor::ProcessGpuJob(const GpuJob& gpu_job) {
  std::string timeline;
  if (gpu_job.timeline_or_key_case() == GpuJob::kTimelineKey) {
    timeline = string_intern_pool[gpu_job.timeline_key()];
  } else {
    timeline = gpu_job.timeline();
  }
  uint64_t timeline_hash = GetStringHashAndSendToListenerIfNecessary(timeline);

  constexpr const char* sw_queue = "sw queue";
  uint64_t sw_queue_key = GetStringHashAndSendToListenerIfNecessary(sw_queue);

  TimerInfo timer_user_to_sched;
  timer_user_to_sched.set_thread_id(gpu_job.tid());
  timer_user_to_sched.set_start(gpu_job.amdgpu_cs_ioctl_time_ns());
  timer_user_to_sched.set_end(gpu_job.amdgpu_sched_run_job_time_ns());
  timer_user_to_sched.set_depth(gpu_job.depth());
  timer_user_to_sched.set_user_data_key(sw_queue_key);
  timer_user_to_sched.set_timeline_hash(timeline_hash);
  timer_user_to_sched.set_processor(-1);
  timer_user_to_sched.set_type(TimerInfo::kGpuActivity);
  capture_listener_->OnTimer(std::move(timer_user_to_sched));

  constexpr const char* hw_queue = "hw queue";
  uint64_t hw_queue_key = GetStringHashAndSendToListenerIfNecessary(hw_queue);

  TimerInfo timer_sched_to_start;
  timer_sched_to_start.set_thread_id(gpu_job.tid());
  timer_sched_to_start.set_start(gpu_job.amdgpu_sched_run_job_time_ns());
  timer_sched_to_start.set_end(gpu_job.gpu_hardware_start_time_ns());
  timer_sched_to_start.set_depth(gpu_job.depth());
  timer_sched_to_start.set_user_data_key(hw_queue_key);
  timer_sched_to_start.set_timeline_hash(timeline_hash);
  timer_sched_to_start.set_processor(-1);
  timer_sched_to_start.set_type(TimerInfo::kGpuActivity);
  capture_listener_->OnTimer(std::move(timer_sched_to_start));

  constexpr const char* hw_execution = "hw execution";
  uint64_t hw_execution_key = GetStringHashAndSendToListenerIfNecessary(hw_execution);

  TimerInfo timer_start_to_finish;
  timer_start_to_finish.set_thread_id(gpu_job.tid());
  timer_start_to_finish.set_start(gpu_job.gpu_hardware_start_time_ns());
  timer_start_to_finish.set_end(gpu_job.dma_fence_signaled_time_ns());
  timer_start_to_finish.set_depth(gpu_job.depth());
  timer_start_to_finish.set_user_data_key(hw_execution_key);
  timer_start_to_finish.set_timeline_hash(timeline_hash);
  timer_start_to_finish.set_processor(-1);
  timer_start_to_finish.set_type(TimerInfo::kGpuActivity);
  capture_listener_->OnTimer(std::move(timer_start_to_finish));
}

void CaptureEventProcessor::ProcessThreadName(const ThreadName& thread_name) {
  capture_listener_->OnThreadName(thread_name.tid(), thread_name.name());
}

void CaptureEventProcessor::ProcessThreadStateSlice(const ThreadStateSlice& thread_state_slice) {
  ThreadStateSliceInfo slice_info;
  slice_info.set_tid(thread_state_slice.tid());
  switch (thread_state_slice.thread_state()) {
    case ThreadStateSlice::kRunning:
      slice_info.set_thread_state(ThreadStateSliceInfo::kRunning);
      break;
    case ThreadStateSlice::kRunnable:
      slice_info.set_thread_state(ThreadStateSliceInfo::kRunnable);
      break;
    case ThreadStateSlice::kInterruptibleSleep:
      slice_info.set_thread_state(ThreadStateSliceInfo::kInterruptibleSleep);
      break;
    case ThreadStateSlice::kUninterruptibleSleep:
      slice_info.set_thread_state(ThreadStateSliceInfo::kUninterruptibleSleep);
      break;
    case ThreadStateSlice::kStopped:
      slice_info.set_thread_state(ThreadStateSliceInfo::kStopped);
      break;
    case ThreadStateSlice::kTraced:
      slice_info.set_thread_state(ThreadStateSliceInfo::kTraced);
      break;
    case ThreadStateSlice::kDead:
      slice_info.set_thread_state(ThreadStateSliceInfo::kDead);
      break;
    case ThreadStateSlice::kZombie:
      slice_info.set_thread_state(ThreadStateSliceInfo::kZombie);
      break;
    case ThreadStateSlice::kParked:
      slice_info.set_thread_state(ThreadStateSliceInfo::kParked);
      break;
    case ThreadStateSlice::kIdle:
      slice_info.set_thread_state(ThreadStateSliceInfo::kIdle);
      break;
    default:
      UNREACHABLE();
  }
  slice_info.set_begin_timestamp_ns(thread_state_slice.begin_timestamp_ns());
  slice_info.set_end_timestamp_ns(thread_state_slice.end_timestamp_ns());
  capture_listener_->OnThreadStateSlice(std::move(slice_info));
}

void CaptureEventProcessor::ProcessAddressInfo(const AddressInfo& address_info) {
  std::string function_name;
  if (address_info.function_name_or_key_case() == AddressInfo::kFunctionNameKey) {
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

  LinuxAddressInfo linux_address_info;
  linux_address_info.set_absolute_address(address_info.absolute_address());
  linux_address_info.set_module_path(map_name);
  linux_address_info.set_function_name(function_name);
  linux_address_info.set_offset_in_function(address_info.offset_in_function());
  capture_listener_->OnAddressInfo(linux_address_info);
}

uint64_t CaptureEventProcessor::GetCallstackHashAndSendToListenerIfNecessary(
    const Callstack& callstack) {
  CallStack cs({callstack.pcs().begin(), callstack.pcs().end()});
  // TODO: Compute the hash without creating the CallStack if not necessary.
  uint64_t hash = cs.GetHash();

  if (!callstack_hashes_seen_.contains(hash)) {
    callstack_hashes_seen_.emplace(hash);
    capture_listener_->OnUniqueCallStack(cs);
  }
  return hash;
}

void CaptureEventProcessor::ProcessInternedTracepointInfo(
    orbit_grpc_protos::InternedTracepointInfo interned_tracepoint_info) {
  if (tracepoint_intern_pool_.contains(interned_tracepoint_info.key())) {
    ERROR("Overwriting InternedTracepointInfo with key %llu", interned_tracepoint_info.key());
  }
  tracepoint_intern_pool_.emplace(interned_tracepoint_info.key(),
                                  std::move(*interned_tracepoint_info.mutable_intern()));
}
void CaptureEventProcessor::ProcessTracepointEvent(
    const orbit_grpc_protos::TracepointEvent& tracepoint_event) {
  CHECK(tracepoint_event.tracepoint_info_or_key_case() ==
        orbit_grpc_protos::TracepointEvent::kTracepointInfoKey);

  const uint64_t& hash = tracepoint_event.tracepoint_info_key();

  CHECK(tracepoint_intern_pool_.contains(hash));

  const auto& tracepoint_info = tracepoint_intern_pool_[hash];

  SendTracepointInfoToListenerIfNecessary(tracepoint_info, hash);
  orbit_client_protos::TracepointEventInfo tracepoint_event_info;
  tracepoint_event_info.set_pid(tracepoint_event.pid());
  tracepoint_event_info.set_tid(tracepoint_event.tid());
  tracepoint_event_info.set_time(tracepoint_event.time());
  tracepoint_event_info.set_cpu(tracepoint_event.cpu());
  tracepoint_event_info.set_tracepoint_info_key(hash);

  capture_listener_->OnTracepointEvent(std::move(tracepoint_event_info));
}

uint64_t CaptureEventProcessor::GetStringHashAndSendToListenerIfNecessary(const std::string& str) {
  uint64_t hash = StringHash(str);
  if (!string_hashes_seen_.contains(hash)) {
    string_hashes_seen_.emplace(hash);
    capture_listener_->OnKeyAndString(hash, str);
  }
  return hash;
}

void CaptureEventProcessor::SendTracepointInfoToListenerIfNecessary(
    const orbit_grpc_protos::TracepointInfo& tracepoint_info, const uint64_t& hash) {
  if (!tracepoint_hashes_seen_.contains(hash)) {
    tracepoint_hashes_seen_.emplace(hash);
    capture_listener_->OnUniqueTracepointInfo(hash, tracepoint_info);
  }
}
