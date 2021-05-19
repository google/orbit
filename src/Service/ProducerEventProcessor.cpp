// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerEventProcessor.h"

#include <absl/container/flat_hash_map.h>

#include "OrbitBase/Logging.h"
#include "capture.pb.h"

namespace orbit_service {

namespace {

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::ApiEvent;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FullGpuJob;
using orbit_grpc_protos::FullTracepointEvent;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuDebugMarker;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;
using orbit_grpc_protos::InternedCallstack;
using orbit_grpc_protos::InternedString;
using orbit_grpc_protos::InternedTracepointInfo;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_grpc_protos::ModuleUpdateEvent;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadNamesSnapshot;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::TracepointEvent;

template <typename T>
class InternPool final {
 public:
  InternPool() = default;

  // Return pair of <id, assigned>, where assigned is true if the entry was assigned a new id
  // and false if returning id for already existing entry.
  std::pair<uint64_t, bool> GetOrAssignId(const T& entry) {
    absl::MutexLock lock(&entry_to_id_mutex_);
    auto it = entry_to_id_.find(entry);
    if (it != entry_to_id_.end()) {
      return std::make_pair(it->second, false);
    }

    uint64_t new_id = id_counter_++;
    auto [unused_it, inserted] = entry_to_id_.insert_or_assign(entry, new_id);
    CHECK(inserted);
    return std::make_pair(new_id, true);
  }

 private:
  uint64_t id_counter_ = 1;  // 0 is reserved for invalid_id
  absl::flat_hash_map<T, uint64_t> entry_to_id_;
  absl::Mutex entry_to_id_mutex_;
};

class ProducerEventProcessorImpl : public ProducerEventProcessor {
 public:
  ProducerEventProcessorImpl() = delete;
  explicit ProducerEventProcessorImpl(CaptureEventBuffer* capture_event_buffer)
      : capture_event_buffer_{capture_event_buffer} {}

  void ProcessEvent(uint64_t producer_id, ProducerCaptureEvent event) override;

 private:
  void ProcessCaptureStarted(CaptureStarted* capture_started);
  void ProcessFullAddressInfo(FullAddressInfo* full_address_info);
  void ProcessFullCallstackSample(FullCallstackSample* full_callstack_sample);
  void ProcessFunctionCall(FunctionCall* function_call);
  void ProcessFullGpuJob(FullGpuJob* full_gpu_job_event);
  void ProcessGpuQueueSubmission(uint64_t producer_id, GpuQueueSubmission* gpu_queue_submission);
  // ProcessInterned* functions remap producer intern_ids to the id space used in the client.
  // They keep track of these mappings in producer_interned_callstack_id_to_client_callstack_id_
  // and producer_interned_string_id_to_client_string_id_.
  void ProcessInternedCallstack(uint64_t producer_id, InternedCallstack* interned_callstack);
  void ProcessCallstackSample(uint64_t producer_id, CallstackSample* callstack_sample);
  void ProcessInternedString(uint64_t producer_id, InternedString* interned_string);
  void ProcessIntrospectionScope(IntrospectionScope* introspection_scope);
  void ProcessModuleUpdateEvent(ModuleUpdateEvent* module_update_event);
  void ProcessModulesSnapshot(ModulesSnapshot* modules_snapshot);
  void ProcessSchedulingSlice(SchedulingSlice* scheduling_slice);
  void ProcessThreadName(ThreadName* thread_name);
  void ProcessThreadNamesSnapshot(ThreadNamesSnapshot* thread_names_snapshot);
  void ProcessThreadStateSlice(ThreadStateSlice* thread_state_slice);
  void ProcessFullTracepointEvent(FullTracepointEvent* full_tracepoint_event);
  void ProcessCGroupMemoryUsage(CGroupMemoryUsage* cgroup_memory_usage);
  void ProcessProcessMemoryUsage(ProcessMemoryUsage* process_memory_usage);
  void ProcessSystemMemoryUsage(SystemMemoryUsage* system_memory_usage);
  void ProcessApiEvent(ApiEvent* api_event);

  void SendInternedStringEvent(uint64_t key, std::string value);

  CaptureEventBuffer* capture_event_buffer_;

  InternPool<std::pair<std::vector<uint64_t>, Callstack::CallstackType>> callstack_pool_;
  InternPool<std::string> string_pool_;
  InternPool<std::pair<std::string, std::string>> tracepoint_pool_;

  // These are mapping InternStrings and InternedCallstacks from producer ids
  // to client ids:
  // <producer_id, producer_callstack_id> -> client_callstack_id
  absl::flat_hash_map<std::pair<uint64_t, uint64_t>, uint64_t>
      producer_interned_callstack_id_to_client_callstack_id_;
  // <producer_id, producer_string_id> -> client_string_id
  absl::flat_hash_map<std::pair<uint64_t, uint64_t>, uint64_t>
      producer_interned_string_id_to_client_string_id_;
};

void ProducerEventProcessorImpl::ProcessFullAddressInfo(FullAddressInfo* full_address_info) {
  auto [function_name_key, function_key_assigned] =
      string_pool_.GetOrAssignId(full_address_info->function_name());
  if (function_key_assigned) {
    SendInternedStringEvent(function_name_key, full_address_info->function_name());
  }

  auto [module_name_key, module_key_assigned] =
      string_pool_.GetOrAssignId(full_address_info->module_name());
  if (module_key_assigned) {
    SendInternedStringEvent(module_name_key, full_address_info->module_name());
  }

  ClientCaptureEvent event;
  AddressInfo* interned_address_info = event.mutable_address_info();
  interned_address_info->set_absolute_address(full_address_info->absolute_address());
  interned_address_info->set_offset_in_function(full_address_info->offset_in_function());
  interned_address_info->set_function_name_key(function_name_key);
  interned_address_info->set_module_name_key(module_name_key);

  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessFunctionCall(FunctionCall* function_call) {
  ClientCaptureEvent event;
  *event.mutable_function_call() = std::move(*function_call);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessFullGpuJob(FullGpuJob* full_gpu_job_event) {
  auto [timeline_key, assigned] = string_pool_.GetOrAssignId(full_gpu_job_event->timeline());
  if (assigned) {
    SendInternedStringEvent(timeline_key, full_gpu_job_event->timeline());
  }

  ClientCaptureEvent event;
  GpuJob* gpu_job_event = event.mutable_gpu_job();
  gpu_job_event->set_pid(full_gpu_job_event->pid());
  gpu_job_event->set_tid(full_gpu_job_event->tid());
  gpu_job_event->set_context(full_gpu_job_event->context());
  gpu_job_event->set_seqno(full_gpu_job_event->seqno());
  gpu_job_event->set_depth(full_gpu_job_event->depth());
  gpu_job_event->set_amdgpu_cs_ioctl_time_ns(full_gpu_job_event->amdgpu_cs_ioctl_time_ns());
  gpu_job_event->set_amdgpu_sched_run_job_time_ns(
      full_gpu_job_event->amdgpu_sched_run_job_time_ns());
  gpu_job_event->set_gpu_hardware_start_time_ns(full_gpu_job_event->gpu_hardware_start_time_ns());
  gpu_job_event->set_dma_fence_signaled_time_ns(full_gpu_job_event->dma_fence_signaled_time_ns());
  gpu_job_event->set_timeline_key(timeline_key);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessGpuQueueSubmission(
    uint64_t producer_id, GpuQueueSubmission* gpu_queue_submission) {
  // Translate debug marker keys
  for (GpuDebugMarker& mutable_marker : *gpu_queue_submission->mutable_completed_markers()) {
    auto it = producer_interned_string_id_to_client_string_id_.find(
        {producer_id, mutable_marker.text_key()});
    CHECK(it != producer_interned_string_id_to_client_string_id_.end());
    mutable_marker.set_text_key(it->second);
  }

  ClientCaptureEvent event;
  *event.mutable_gpu_queue_submission() = std::move(*gpu_queue_submission);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessFullCallstackSample(
    FullCallstackSample* full_callstack_sample) {
  Callstack* callstack = full_callstack_sample->mutable_callstack();
  std::pair<std::vector<uint64_t>, Callstack::CallstackType> callstack_data{
      {callstack->pcs().begin(), callstack->pcs().end()}, callstack->type()};
  auto [callstack_id, assigned] = callstack_pool_.GetOrAssignId(callstack_data);

  if (assigned) {
    ClientCaptureEvent interned_callstack_event;
    interned_callstack_event.mutable_interned_callstack()->set_key(callstack_id);
    *interned_callstack_event.mutable_interned_callstack()->mutable_intern() =
        std::move(*callstack);
    capture_event_buffer_->AddEvent(std::move(interned_callstack_event));
  }

  ClientCaptureEvent callstack_sample_event;
  CallstackSample* callstack_sample = callstack_sample_event.mutable_callstack_sample();
  callstack_sample->set_pid(full_callstack_sample->pid());
  callstack_sample->set_tid(full_callstack_sample->tid());
  callstack_sample->set_timestamp_ns(full_callstack_sample->timestamp_ns());
  callstack_sample->set_callstack_id(callstack_id);
  capture_event_buffer_->AddEvent(std::move(callstack_sample_event));
}

void ProducerEventProcessorImpl::ProcessInternedCallstack(uint64_t producer_id,
                                                          InternedCallstack* interned_callstack) {
  // TODO(b/180235290): replace with error message
  CHECK(!producer_interned_callstack_id_to_client_callstack_id_.contains(
      {producer_id, interned_callstack->key()}));

  std::pair<std::vector<uint64_t>, Callstack::CallstackType> callstack_data{
      {interned_callstack->intern().pcs().begin(), interned_callstack->intern().pcs().end()},
      interned_callstack->intern().type()};
  auto [interned_callstack_id, assigned] = callstack_pool_.GetOrAssignId(callstack_data);

  producer_interned_callstack_id_to_client_callstack_id_.insert_or_assign(
      {producer_id, interned_callstack->key()}, interned_callstack_id);

  if (!assigned) {
    return;
  }

  // If this is first time we see it -> send it over with client_id
  interned_callstack->set_key(interned_callstack_id);
  ClientCaptureEvent event;
  *event.mutable_interned_callstack() = std::move(*interned_callstack);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessCallstackSample(uint64_t producer_id,
                                                        CallstackSample* callstack_sample) {
  // translate producer id to client id
  auto it = producer_interned_callstack_id_to_client_callstack_id_.find(
      {producer_id, callstack_sample->callstack_id()});
  // TODO(b/180235290): replace with error message
  CHECK(it != producer_interned_callstack_id_to_client_callstack_id_.end());
  callstack_sample->set_callstack_id(it->second);

  ClientCaptureEvent event;
  *event.mutable_callstack_sample() = std::move(*callstack_sample);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessInternedString(uint64_t producer_id,
                                                       InternedString* interned_string) {
  // TODO(b/180235290): replace with error message
  CHECK(!producer_interned_string_id_to_client_string_id_.contains(
      {producer_id, interned_string->key()}));

  auto [client_string_id, assigned] = string_pool_.GetOrAssignId(interned_string->intern());
  producer_interned_string_id_to_client_string_id_.insert_or_assign(
      {producer_id, interned_string->key()}, client_string_id);

  if (!assigned) {
    return;
  }

  interned_string->set_key(client_string_id);

  ClientCaptureEvent event;
  *event.mutable_interned_string() = std::move(*interned_string);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessIntrospectionScope(
    IntrospectionScope* introspection_scope) {
  ClientCaptureEvent event;
  *event.mutable_introspection_scope() = std::move(*introspection_scope);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessModuleUpdateEvent(
    orbit_grpc_protos::ModuleUpdateEvent* module_update_event) {
  ClientCaptureEvent event;
  *event.mutable_module_update_event() = std::move(*module_update_event);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessModulesSnapshot(ModulesSnapshot* modules_snapshot) {
  ClientCaptureEvent event;
  *event.mutable_modules_snapshot() = std::move(*modules_snapshot);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessCaptureStarted(CaptureStarted* capture_started) {
  ClientCaptureEvent event;
  *event.mutable_capture_started() = std::move(*capture_started);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessSchedulingSlice(SchedulingSlice* scheduling_slice) {
  ClientCaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(*scheduling_slice);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessThreadName(ThreadName* thread_name) {
  ClientCaptureEvent event;
  *event.mutable_thread_name() = std::move(*thread_name);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessThreadNamesSnapshot(
    ThreadNamesSnapshot* thread_names_snapshot) {
  ClientCaptureEvent event;
  *event.mutable_thread_names_snapshot() = std::move(*thread_names_snapshot);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessThreadStateSlice(ThreadStateSlice* thread_state_slice) {
  ClientCaptureEvent event;
  *event.mutable_thread_state_slice() = std::move(*thread_state_slice);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessFullTracepointEvent(
    FullTracepointEvent* full_tracepoint_event) {
  auto [tracepoint_key, assigned] =
      tracepoint_pool_.GetOrAssignId({full_tracepoint_event->tracepoint_info().category(),
                                      full_tracepoint_event->tracepoint_info().name()});
  if (assigned) {
    ClientCaptureEvent event;
    InternedTracepointInfo* interned_tracepoint_info = event.mutable_interned_tracepoint_info();
    interned_tracepoint_info->set_key(tracepoint_key);
    *interned_tracepoint_info->mutable_intern() = full_tracepoint_event->tracepoint_info();
    capture_event_buffer_->AddEvent(std::move(event));
  }

  ClientCaptureEvent event;
  TracepointEvent* tracepoint_event = event.mutable_tracepoint_event();
  tracepoint_event->set_pid(full_tracepoint_event->pid());
  tracepoint_event->set_tid(full_tracepoint_event->tid());
  tracepoint_event->set_timestamp_ns(full_tracepoint_event->timestamp_ns());
  tracepoint_event->set_cpu(full_tracepoint_event->cpu());
  tracepoint_event->set_tracepoint_info_key(tracepoint_key);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessCGroupMemoryUsage(CGroupMemoryUsage* cgroup_memory_usage) {
  ClientCaptureEvent event;
  *event.mutable_cgroup_memory_usage() = std::move(*cgroup_memory_usage);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessProcessMemoryUsage(
    ProcessMemoryUsage* process_memory_usage) {
  ClientCaptureEvent event;
  *event.mutable_process_memory_usage() = std::move(*process_memory_usage);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessSystemMemoryUsage(SystemMemoryUsage* system_memory_usage) {
  ClientCaptureEvent event;
  *event.mutable_system_memory_usage() = std::move(*system_memory_usage);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessApiEvent(ApiEvent* api_event) {
  ClientCaptureEvent event;
  *event.mutable_api_event() = std::move(*api_event);
  capture_event_buffer_->AddEvent(std::move(event));
}

void ProducerEventProcessorImpl::ProcessEvent(uint64_t producer_id, ProducerCaptureEvent event) {
  switch (event.event_case()) {
    case ProducerCaptureEvent::kCaptureStarted:
      ProcessCaptureStarted(event.mutable_capture_started());
      break;
    case ProducerCaptureEvent::kInternedCallstack:
      ProcessInternedCallstack(producer_id, event.mutable_interned_callstack());
      break;
    case ProducerCaptureEvent::kSchedulingSlice:
      ProcessSchedulingSlice(event.mutable_scheduling_slice());
      break;
    case ProducerCaptureEvent::kCallstackSample:
      ProcessCallstackSample(producer_id, event.mutable_callstack_sample());
      break;
    case ProducerCaptureEvent::kFullCallstackSample:
      ProcessFullCallstackSample(event.mutable_full_callstack_sample());
      break;
    case ProducerCaptureEvent::kFullTracepointEvent:
      ProcessFullTracepointEvent(event.mutable_full_tracepoint_event());
      break;
    case ProducerCaptureEvent::kFunctionCall:
      ProcessFunctionCall(event.mutable_function_call());
      break;
    case ProducerCaptureEvent::kInternedString:
      ProcessInternedString(producer_id, event.mutable_interned_string());
      break;
    case ProducerCaptureEvent::kFullGpuJob:
      ProcessFullGpuJob(event.mutable_full_gpu_job());
      break;
    case ProducerCaptureEvent::kGpuQueueSubmission:
      ProcessGpuQueueSubmission(producer_id, event.mutable_gpu_queue_submission());
      break;
    case ProducerCaptureEvent::kThreadName:
      ProcessThreadName(event.mutable_thread_name());
      break;
    case ProducerCaptureEvent::kThreadNamesSnapshot:
      ProcessThreadNamesSnapshot(event.mutable_thread_names_snapshot());
      break;
    case ProducerCaptureEvent::kThreadStateSlice:
      ProcessThreadStateSlice(event.mutable_thread_state_slice());
      break;
    case ProducerCaptureEvent::kFullAddressInfo:
      ProcessFullAddressInfo(event.mutable_full_address_info());
      break;
    case ProducerCaptureEvent::kIntrospectionScope:
      ProcessIntrospectionScope(event.mutable_introspection_scope());
      break;
    case ProducerCaptureEvent::kModuleUpdateEvent:
      ProcessModuleUpdateEvent(event.mutable_module_update_event());
      break;
    case ProducerCaptureEvent::kModulesSnapshot:
      ProcessModulesSnapshot(event.mutable_modules_snapshot());
      break;
    case ProducerCaptureEvent::kCgroupMemoryUsage:
      ProcessCGroupMemoryUsage(event.mutable_cgroup_memory_usage());
      break;
    case ProducerCaptureEvent::kProcessMemoryUsage:
      ProcessProcessMemoryUsage(event.mutable_process_memory_usage());
      break;
    case ProducerCaptureEvent::kSystemMemoryUsage:
      ProcessSystemMemoryUsage(event.mutable_system_memory_usage());
      break;
    case ProducerCaptureEvent::kApiEvent:
      ProcessApiEvent(event.mutable_api_event());
      break;
    case ProducerCaptureEvent::EVENT_NOT_SET:
      UNREACHABLE();
  }
}

void ProducerEventProcessorImpl::SendInternedStringEvent(uint64_t key, std::string value) {
  ClientCaptureEvent event;
  InternedString* interned_string = event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(std::move(value));
  capture_event_buffer_->AddEvent(std::move(event));
}

}  // namespace

std::unique_ptr<ProducerEventProcessor> ProducerEventProcessor::Create(
    CaptureEventBuffer* capture_event_buffer) {
  return std::make_unique<ProducerEventProcessorImpl>(capture_event_buffer);
}

}  // namespace orbit_service
