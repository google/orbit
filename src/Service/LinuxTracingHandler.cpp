// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingHandler.h"

#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_cat.h>
#include <absl/synchronization/mutex.h>
#include <unistd.h>

#include <utility>

#include "OrbitBase/Logging.h"
#include "llvm/Demangle/Demangle.h"

namespace orbit_service {

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::InternedCallstackSample;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;

constexpr uint64_t kLinuxTracingProducerId = 0;

void LinuxTracingHandler::Start(CaptureOptions capture_options) {
  CHECK(tracer_ == nullptr);
  bool enable_introspection = capture_options.enable_introspection();

  tracer_ = std::make_unique<orbit_linux_tracing::Tracer>(std::move(capture_options));
  tracer_->SetListener(this);
  tracer_->Start();

  if (enable_introspection) {
    SetupIntrospection();
  }
}

void LinuxTracingHandler::SetupIntrospection() {
  orbit_tracing_listener_ =
      std::make_unique<orbit_base::TracingListener>([this](const orbit_base::TracingScope& scope) {
        IntrospectionScope introspection_scope;
        introspection_scope.set_pid(getpid());
        introspection_scope.set_tid(scope.tid);
        introspection_scope.set_duration_ns(scope.end - scope.begin);
        introspection_scope.set_end_timestamp_ns(scope.end);
        introspection_scope.set_depth(scope.depth);
        introspection_scope.mutable_registers()->Reserve(6);
        introspection_scope.add_registers(scope.encoded_event.args[0]);
        introspection_scope.add_registers(scope.encoded_event.args[1]);
        introspection_scope.add_registers(scope.encoded_event.args[2]);
        introspection_scope.add_registers(scope.encoded_event.args[3]);
        introspection_scope.add_registers(scope.encoded_event.args[4]);
        introspection_scope.add_registers(scope.encoded_event.args[5]);
        OnIntrospectionScope(introspection_scope);
      });
}

void LinuxTracingHandler::Stop() {
  CHECK(tracer_ != nullptr);
  tracer_->Stop();
  tracer_.reset();
}

void LinuxTracingHandler::OnSchedulingSlice(SchedulingSlice scheduling_slice) {
  ProducerCaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(scheduling_slice);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnCallstackSample(FullCallstackSample callstack_sample) {
  ProducerCaptureEvent event;
  *event.mutable_full_callstack_sample() = std::move(callstack_sample);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnFunctionCall(FunctionCall function_call) {
  ProducerCaptureEvent event;
  *event.mutable_function_call() = std::move(function_call);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnIntrospectionScope(
    orbit_grpc_protos::IntrospectionScope introspection_scope) {
  ProducerCaptureEvent event;
  *event.mutable_introspection_scope() = std::move(introspection_scope);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnGpuJob(GpuJob gpu_job) {
  CHECK(gpu_job.timeline_or_key_case() == GpuJob::kTimeline);
  ProducerCaptureEvent event;
  *event.mutable_gpu_job() = std::move(gpu_job);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnThreadName(ThreadName thread_name) {
  ProducerCaptureEvent event;
  *event.mutable_thread_name() = std::move(thread_name);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnThreadStateSlice(ThreadStateSlice thread_state_slice) {
  ProducerCaptureEvent event;
  *event.mutable_thread_state_slice() = std::move(thread_state_slice);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnAddressInfo(AddressInfo address_info) {
  CHECK(address_info.function_name_or_key_case() == AddressInfo::kFunctionName);
  CHECK(address_info.map_name_or_key_case() == AddressInfo::kMapName);

  ProducerCaptureEvent event;
  *event.mutable_address_info() = std::move(address_info);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnTracepointEvent(
    orbit_grpc_protos::FullTracepointEvent tracepoint_event) {
  ProducerCaptureEvent event;
  *event.mutable_full_tracepoint_event() = std::move(tracepoint_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void LinuxTracingHandler::OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) {
  ProducerCaptureEvent event;
  *event.mutable_module_update_event() = std::move(module_update_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

}  // namespace orbit_service
