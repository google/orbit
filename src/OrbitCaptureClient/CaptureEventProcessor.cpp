// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureEventProcessor.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <google/protobuf/stubs/port.h>

#include <utility>

#include "CoreUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientData/Callstack.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::Color;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::ThreadStateSliceInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureEvent;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;
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
    case CaptureEvent::kGpuQueueSubmission:
      ProcessGpuQueueSubmission(event.gpu_queue_submission());
      break;
    case CaptureEvent::kModuleUpdateEvent:
      // TODO (http://b/168797897): Process module update events
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

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, scheduling_slice.in_timestamp_ns());

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

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, callstack_sample.timestamp_ns());

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

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, function_call.begin_timestamp_ns());

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

  begin_capture_time_ns_ =
      std::min(begin_capture_time_ns_, introspection_scope.begin_timestamp_ns());

  capture_listener_->OnTimer(timer_info);
}

void CaptureEventProcessor::ProcessInternedString(InternedString interned_string) {
  if (string_intern_pool_.contains(interned_string.key())) {
    ERROR("Overwriting InternedString with key %llu", interned_string.key());
  }
  string_intern_pool_.emplace(interned_string.key(), std::move(*interned_string.mutable_intern()));
}

void CaptureEventProcessor::ProcessGpuJob(const GpuJob& gpu_job) {
  std::string timeline;
  if (gpu_job.timeline_or_key_case() == GpuJob::kTimelineKey) {
    timeline = string_intern_pool_[gpu_job.timeline_key()];
  } else {
    timeline = gpu_job.timeline();
  }
  uint64_t timeline_hash = GetStringHashAndSendToListenerIfNecessary(timeline);

  int32_t thread_id = gpu_job.tid();
  uint64_t amdgpu_cs_ioctl_time_ns = gpu_job.amdgpu_cs_ioctl_time_ns();

  constexpr const char* sw_queue = "sw queue";
  uint64_t sw_queue_key = GetStringHashAndSendToListenerIfNecessary(sw_queue);

  TimerInfo timer_user_to_sched;
  timer_user_to_sched.set_thread_id(thread_id);
  timer_user_to_sched.set_start(amdgpu_cs_ioctl_time_ns);
  timer_user_to_sched.set_end(gpu_job.amdgpu_sched_run_job_time_ns());
  timer_user_to_sched.set_depth(gpu_job.depth());
  timer_user_to_sched.set_user_data_key(sw_queue_key);
  timer_user_to_sched.set_timeline_hash(timeline_hash);
  timer_user_to_sched.set_processor(-1);
  timer_user_to_sched.set_type(TimerInfo::kGpuActivity);

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, gpu_job.amdgpu_cs_ioctl_time_ns());

  capture_listener_->OnTimer(std::move(timer_user_to_sched));

  constexpr const char* hw_queue = "hw queue";
  uint64_t hw_queue_key = GetStringHashAndSendToListenerIfNecessary(hw_queue);

  TimerInfo timer_sched_to_start;
  timer_sched_to_start.set_thread_id(thread_id);
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
  timer_start_to_finish.set_thread_id(thread_id);
  timer_start_to_finish.set_start(gpu_job.gpu_hardware_start_time_ns());
  timer_start_to_finish.set_end(gpu_job.dma_fence_signaled_time_ns());
  timer_start_to_finish.set_depth(gpu_job.depth());
  timer_start_to_finish.set_user_data_key(hw_execution_key);
  timer_start_to_finish.set_timeline_hash(timeline_hash);
  timer_start_to_finish.set_processor(-1);
  timer_start_to_finish.set_type(TimerInfo::kGpuActivity);
  capture_listener_->OnTimer(std::move(timer_start_to_finish));

  const GpuQueueSubmission* matching_gpu_submission =
      FindMatchingGpuQueueSubmission(thread_id, amdgpu_cs_ioctl_time_ns);

  // If we haven't found the matching "GpuSubmission" or the submission contains "begin" markers
  // (which might have the "end" markers in a later submission), we save the "GpuJob" for later.
  // Note that as soon as all "begin" markers have been processed, the "GpuJob" will be deleted
  // again.
  if (matching_gpu_submission == nullptr || matching_gpu_submission->num_begin_markers() > 0) {
    tid_to_submission_time_to_gpu_job_[thread_id][amdgpu_cs_ioctl_time_ns] = gpu_job;
  }
  if (matching_gpu_submission == nullptr) {
    return;
  }

  ProcessGpuQueueSubmissionWithMatchingGpuJob(*matching_gpu_submission, gpu_job);

  uint64_t post_submission_cpu_timestamp =
      matching_gpu_submission->meta_info().post_submission_cpu_timestamp();

  if (!HasUnprocessedBeginMarkers(thread_id, post_submission_cpu_timestamp)) {
    DeleteSavedGpuSubmission(thread_id, post_submission_cpu_timestamp);
  }
}

void CaptureEventProcessor::ProcessGpuQueueSubmission(
    const GpuQueueSubmission& gpu_queue_submission) {
  int32_t thread_id = gpu_queue_submission.meta_info().tid();
  uint64_t pre_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().pre_submission_cpu_timestamp();
  uint64_t post_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().post_submission_cpu_timestamp();
  const GpuJob* matching_gpu_job =
      FindMatchingGpuJob(thread_id, pre_submission_cpu_timestamp, post_submission_cpu_timestamp);

  // If we haven't found the matching "GpuSubmission" or the submission contains "begin" markers
  // (which might have the "end" markers in a later submission), we save the "GpuSubmission" for
  // later.
  // Note that as soon as all "begin" markers have been processed, the "GpuSubmission" will be
  // deleted again.
  if (matching_gpu_job == nullptr || gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_gpu_submission_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission;
  }
  if (gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_num_begin_markers_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission.num_begin_markers();
  }
  if (matching_gpu_job == nullptr) {
    return;
  }

  ProcessGpuQueueSubmissionWithMatchingGpuJob(gpu_queue_submission, *matching_gpu_job);

  if (!HasUnprocessedBeginMarkers(thread_id, post_submission_cpu_timestamp)) {
    DeleteSavedGpuJob(thread_id, matching_gpu_job->amdgpu_cs_ioctl_time_ns());
  }
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

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, slice_info.begin_timestamp_ns());

  capture_listener_->OnThreadStateSlice(std::move(slice_info));
}

void CaptureEventProcessor::ProcessAddressInfo(const AddressInfo& address_info) {
  std::string function_name;
  if (address_info.function_name_or_key_case() == AddressInfo::kFunctionNameKey) {
    function_name = string_intern_pool_[address_info.function_name_key()];
  } else {
    function_name = address_info.function_name();
  }

  std::string map_name;
  if (address_info.map_name_or_key_case() == AddressInfo::kMapNameKey) {
    map_name = string_intern_pool_[address_info.map_name_key()];
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

  begin_capture_time_ns_ = std::min(begin_capture_time_ns_, tracepoint_event.time());

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

// Finds the GpuQueueSubmission that fully contains the given given timestamp and happened on the
// given thread id. Returns `nullptr` if there is no such submission.
const GpuQueueSubmission* CaptureEventProcessor::FindMatchingGpuQueueSubmission(
    int32_t thread_id, uint64_t submit_time) {
  const auto& post_submission_time_to_gpu_submission_it =
      tid_to_post_submission_time_to_gpu_submission_.find(thread_id);
  if (post_submission_time_to_gpu_submission_it ==
      tid_to_post_submission_time_to_gpu_submission_.end()) {
    return nullptr;
  }

  const auto& post_submission_time_to_gpu_submission =
      post_submission_time_to_gpu_submission_it->second;

  // Find the first Gpu submission with a "post submission" timestamp greater or equal to the Gpu
  // job's timestamp. If the "pre submission" timestamp is not greater (i.e. less or equal) than the
  // job's timestamp, we have found the matching submission.
  auto lower_bound_gpu_submission_it =
      post_submission_time_to_gpu_submission.lower_bound(submit_time);
  if (lower_bound_gpu_submission_it == post_submission_time_to_gpu_submission.end()) {
    return nullptr;
  }
  const GpuQueueSubmission* matching_gpu_submission = &lower_bound_gpu_submission_it->second;

  if (matching_gpu_submission->meta_info().pre_submission_cpu_timestamp() > submit_time) {
    return nullptr;
  }

  return matching_gpu_submission;
}

const GpuJob* CaptureEventProcessor::FindMatchingGpuJob(int32_t thread_id,
                                                        uint64_t pre_submission_cpu_timestamp,
                                                        uint64_t post_submission_cpu_timestamp) {
  const auto& submission_time_to_gpu_job_it = tid_to_submission_time_to_gpu_job_.find(thread_id);
  if (submission_time_to_gpu_job_it == tid_to_submission_time_to_gpu_job_.end()) {
    return nullptr;
  }

  const auto& submission_time_to_gpu_job = submission_time_to_gpu_job_it->second;

  // Find the first Gpu job that has a timestamp greater or equal to the "pre submission" timestamp:
  auto gpu_job_matching_pre_submission_it =
      submission_time_to_gpu_job.lower_bound(pre_submission_cpu_timestamp);
  if (gpu_job_matching_pre_submission_it == submission_time_to_gpu_job.end()) {
    return nullptr;
  }

  // Find the first Gpu job that has a timestamp greater to the "post submission" timestamp
  // (which would be the next job) and decrease the iterator by one.
  auto gpu_job_matching_post_submission_it =
      submission_time_to_gpu_job.upper_bound(post_submission_cpu_timestamp);
  if (gpu_job_matching_post_submission_it == submission_time_to_gpu_job.begin()) {
    return nullptr;
  }
  --gpu_job_matching_post_submission_it;

  if (&gpu_job_matching_pre_submission_it->second != &gpu_job_matching_post_submission_it->second) {
    return nullptr;
  }

  return &gpu_job_matching_pre_submission_it->second;
}

void CaptureEventProcessor::ProcessGpuQueueSubmissionWithMatchingGpuJob(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job) {
  std::string timeline;
  if (matching_gpu_job.timeline_or_key_case() == GpuJob::kTimelineKey) {
    CHECK(string_intern_pool_.contains(matching_gpu_job.timeline_key()));
    timeline = string_intern_pool_.at(matching_gpu_job.timeline_key());
  } else {
    timeline = matching_gpu_job.timeline();
  }
  uint64_t timeline_hash = GetStringHashAndSendToListenerIfNecessary(timeline);

  std::optional<GpuCommandBuffer> first_command_buffer =
      ExtractFirstCommandBuffer(gpu_queue_submission);

  ProcessGpuCommandBuffers(gpu_queue_submission, matching_gpu_job, first_command_buffer,
                           timeline_hash);

  ProcessGpuDebugMarkers(gpu_queue_submission, matching_gpu_job, first_command_buffer, timeline);
}

bool CaptureEventProcessor::HasUnprocessedBeginMarkers(int32_t thread_id,
                                                       uint64_t post_submission_timestamp) const {
  if (!tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id)) {
    return false;
  }
  if (!tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).contains(
          post_submission_timestamp)) {
    return false;
  }
  CHECK(tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).at(
            post_submission_timestamp) > 0);
  return true;
}

void CaptureEventProcessor::DecrementUnprocessedBeginMarkers(int32_t thread_id,
                                                             uint64_t submission_timestamp,
                                                             uint64_t post_submission_timestamp) {
  CHECK(tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id));
  auto& post_submission_time_to_num_begin_markers =
      tid_to_post_submission_time_to_num_begin_markers_.at(thread_id);
  CHECK(post_submission_time_to_num_begin_markers.contains(post_submission_timestamp));
  uint64_t new_num = post_submission_time_to_num_begin_markers.at(post_submission_timestamp) - 1;
  post_submission_time_to_num_begin_markers[post_submission_timestamp] = new_num;
  if (new_num == 0) {
    post_submission_time_to_num_begin_markers.erase(post_submission_timestamp);
    if (post_submission_time_to_num_begin_markers.empty()) {
      tid_to_post_submission_time_to_num_begin_markers_.erase(thread_id);
      DeleteSavedGpuJob(thread_id, submission_timestamp);
      DeleteSavedGpuSubmission(thread_id, post_submission_timestamp);
    }
  }
}

void CaptureEventProcessor::DeleteSavedGpuJob(int32_t thread_id, uint64_t submission_timestamp) {
  if (!tid_to_submission_time_to_gpu_job_.contains(thread_id)) {
    return;
  }
  auto& submission_time_to_gpu_job = tid_to_submission_time_to_gpu_job_.at(thread_id);
  submission_time_to_gpu_job.erase(submission_timestamp);
  if (submission_time_to_gpu_job.empty()) {
    tid_to_submission_time_to_gpu_job_.erase(thread_id);
  }
}
void CaptureEventProcessor::DeleteSavedGpuSubmission(int32_t thread_id,
                                                     uint64_t post_submission_timestamp) {
  if (!tid_to_post_submission_time_to_gpu_submission_.contains(thread_id)) {
    return;
  }
  auto& post_submission_time_to_gpu_submission =
      tid_to_post_submission_time_to_gpu_submission_.at(thread_id);
  post_submission_time_to_gpu_submission.erase(post_submission_timestamp);
  if (post_submission_time_to_gpu_submission.empty()) {
    tid_to_post_submission_time_to_gpu_submission_.erase(thread_id);
  }
}

void CaptureEventProcessor::ProcessGpuCommandBuffers(
    const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
    const orbit_grpc_protos::GpuJob& matching_gpu_job,
    const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
    uint64_t timeline_hash) {
  constexpr const char* kCommandBufferLabel = "command buffer";
  uint64_t command_buffer_text_key = GetStringHashAndSendToListenerIfNecessary(kCommandBufferLabel);

  int32_t thread_id = gpu_queue_submission.meta_info().tid();

  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      CHECK(first_command_buffer != std::nullopt);
      TimerInfo command_buffer_timer;
      if (command_buffer.begin_gpu_timestamp_ns() != 0) {
        command_buffer_timer.set_start(command_buffer.begin_gpu_timestamp_ns() -
                                       first_command_buffer->begin_gpu_timestamp_ns() +
                                       matching_gpu_job.gpu_hardware_start_time_ns());
      } else {
        command_buffer_timer.set_start(begin_capture_time_ns_);
      }

      command_buffer_timer.set_end(command_buffer.end_gpu_timestamp_ns() -
                                   first_command_buffer->begin_gpu_timestamp_ns() +
                                   matching_gpu_job.gpu_hardware_start_time_ns());
      command_buffer_timer.set_depth(matching_gpu_job.depth());
      command_buffer_timer.set_timeline_hash(timeline_hash);
      command_buffer_timer.set_processor(-1);
      command_buffer_timer.set_thread_id(thread_id);
      command_buffer_timer.set_type(TimerInfo::kGpuCommandBuffer);
      command_buffer_timer.set_user_data_key(command_buffer_text_key);
      capture_listener_->OnTimer(command_buffer_timer);
    }
  }
}

void CaptureEventProcessor::ProcessGpuDebugMarkers(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const std::optional<GpuCommandBuffer>& first_command_buffer, const std::string& timeline) {
  if (gpu_queue_submission.completed_markers_size() == 0) {
    return;
  }
  std::string timeline_marker = timeline + "_marker";
  uint64_t timeline_marker_hash = GetStringHashAndSendToListenerIfNecessary(timeline_marker);

  const auto& submission_meta_info = gpu_queue_submission.meta_info();
  const int32_t submission_thread_id = submission_meta_info.tid();
  uint64_t submission_pre_submission_cpu_timestamp =
      submission_meta_info.pre_submission_cpu_timestamp();
  uint64_t submission_post_submission_cpu_timestamp =
      submission_meta_info.post_submission_cpu_timestamp();

  for (const auto& completed_marker : gpu_queue_submission.completed_markers()) {
    CHECK(first_command_buffer != std::nullopt);
    TimerInfo marker_timer;

    // If we've recorded the submission that contains the begin marker, we'll retrieve this
    // submission from our mappings, and set the markers begin time accordingly.
    // Otherwise, we will use the capture start time as begin.
    if (completed_marker.has_begin_marker()) {
      const auto& begin_marker_info = completed_marker.begin_marker();
      const auto& begin_marker_meta_info = begin_marker_info.meta_info();
      const int32_t begin_marker_thread_id = begin_marker_meta_info.tid();
      uint64_t begin_marker_post_submission_cpu_timestamp =
          begin_marker_meta_info.post_submission_cpu_timestamp();
      uint64_t begin_marker_pre_submission_cpu_timestamp =
          begin_marker_meta_info.pre_submission_cpu_timestamp();

      // Note that the "begin" and "end" of a debug marker may not happen on the same submission.
      // For those cases, we save the meta information of the "begin" marker's submission in the
      // marker information. We will always send the marker on the "end" marker's submission though.
      // So let's check if the meta data is the same as the current submission (i.e. the marker
      // begins and ends on this submission). If this is the case, use that submission. Otherwise,
      // find the submission that matches the given meta data (that we must have received before,
      // and must still be saved).
      const GpuQueueSubmission* matching_begin_submission = nullptr;
      if (submission_pre_submission_cpu_timestamp == begin_marker_pre_submission_cpu_timestamp &&
          submission_post_submission_cpu_timestamp == begin_marker_post_submission_cpu_timestamp &&
          submission_thread_id == begin_marker_thread_id) {
        matching_begin_submission = &gpu_queue_submission;
      } else {
        matching_begin_submission = FindMatchingGpuQueueSubmission(
            begin_marker_thread_id, begin_marker_post_submission_cpu_timestamp);
      }

      // Note that we receive submissions of a single queue in order (by CPU submission time). So if
      // there is no matching "begin submission", the "begin" was submitted before the "end" and
      // we lost the record of the "begin submission" (which should not happen).
      CHECK(matching_begin_submission != nullptr);

      std::optional<GpuCommandBuffer> begin_submission_first_command_buffer =
          ExtractFirstCommandBuffer(*matching_begin_submission);
      CHECK(begin_submission_first_command_buffer.has_value());

      const GpuJob* matching_begin_job = FindMatchingGpuJob(
          begin_marker_thread_id, begin_marker_meta_info.pre_submission_cpu_timestamp(),
          begin_marker_post_submission_cpu_timestamp);
      CHECK(matching_begin_job != nullptr);

      // Convert the GPU time to CPU time, based on the CPU time of the HW execution begin and the
      // GPU timestamp of the begin of the first command buffer. Note that we will assume that the
      // first command buffer starts execution right away as an approximation.
      marker_timer.set_start(completed_marker.begin_marker().gpu_timestamp_ns() +
                             matching_begin_job->gpu_hardware_start_time_ns() -
                             begin_submission_first_command_buffer->begin_gpu_timestamp_ns());
      if (begin_marker_thread_id == gpu_queue_submission.meta_info().tid()) {
        marker_timer.set_thread_id(begin_marker_thread_id);
      }

      DecrementUnprocessedBeginMarkers(begin_marker_thread_id,
                                       matching_begin_job->amdgpu_cs_ioctl_time_ns(),
                                       begin_marker_post_submission_cpu_timestamp);
    } else {
      marker_timer.set_start(begin_capture_time_ns_);
      marker_timer.set_thread_id(-1);
    }

    marker_timer.set_depth(completed_marker.depth());
    marker_timer.set_timeline_hash(timeline_marker_hash);
    marker_timer.set_processor(-1);
    marker_timer.set_type(TimerInfo::kGpuDebugMarker);
    marker_timer.set_end(completed_marker.end_gpu_timestamp_ns() -
                         first_command_buffer->begin_gpu_timestamp_ns() +
                         matching_gpu_job.gpu_hardware_start_time_ns());

    CHECK(string_intern_pool_.contains(completed_marker.text_key()));
    const std::string& text = string_intern_pool_.at(completed_marker.text_key());
    uint64_t text_key = GetStringHashAndSendToListenerIfNecessary(text);

    if (completed_marker.has_color()) {
      Color* color = marker_timer.mutable_color();
      color->set_red(static_cast<uint32_t>(completed_marker.color().red() * 255));
      color->set_green(static_cast<uint32_t>(completed_marker.color().green() * 255));
      color->set_blue(static_cast<uint32_t>(completed_marker.color().blue() * 255));
      color->set_alpha(static_cast<uint32_t>(completed_marker.color().alpha() * 255));
    }
    marker_timer.set_user_data_key(text_key);
    capture_listener_->OnTimer(marker_timer);
  }
}

std::optional<orbit_grpc_protos::GpuCommandBuffer> CaptureEventProcessor::ExtractFirstCommandBuffer(
    const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission) {
  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      return command_buffer;
    }
  }
  return std::nullopt;
}
