// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingHandler.h"

#include "absl/flags/flag.h"
#include "llvm/Demangle/Demangle.h"

namespace orbit_service {

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureEvent;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;

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
        introspection_scope.set_begin_timestamp_ns(scope.begin);
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
  CaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(scheduling_slice);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnCallstackSample(CallstackSample callstack_sample) {
  CHECK(callstack_sample.callstack_or_key_case() == CallstackSample::kCallstack);
  callstack_sample.set_callstack_key(
      InternCallstackIfNecessaryAndGetKey(callstack_sample.callstack()));

  CaptureEvent event;
  *event.mutable_callstack_sample() = std::move(callstack_sample);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnFunctionCall(FunctionCall function_call) {
  CaptureEvent event;
  *event.mutable_function_call() = std::move(function_call);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnIntrospectionScope(
    orbit_grpc_protos::IntrospectionScope introspection_scope) {
  CaptureEvent event;
  *event.mutable_introspection_scope() = std::move(introspection_scope);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnGpuJob(GpuJob gpu_job) {
  CHECK(gpu_job.timeline_or_key_case() == GpuJob::kTimeline);
  gpu_job.set_timeline_key(
      InternStringIfNecessaryAndGetKey(std::move(*gpu_job.mutable_timeline())));

  CaptureEvent event;
  *event.mutable_gpu_job() = std::move(gpu_job);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnThreadName(ThreadName thread_name) {
  CaptureEvent event;
  *event.mutable_thread_name() = std::move(thread_name);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnThreadStateSlice(ThreadStateSlice thread_state_slice) {
  CaptureEvent event;
  *event.mutable_thread_state_slice() = std::move(thread_state_slice);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnAddressInfo(AddressInfo address_info) {
  {
    absl::MutexLock lock{&addresses_seen_mutex_};
    if (addresses_seen_.contains(address_info.absolute_address())) {
      return;
    }
    addresses_seen_.emplace(address_info.absolute_address());
  }

  CHECK(address_info.function_name_or_key_case() == AddressInfo::kFunctionName);
  address_info.set_function_name_key(
      InternStringIfNecessaryAndGetKey(llvm::demangle(address_info.function_name())));
  CHECK(address_info.map_name_or_key_case() == AddressInfo::kMapName);
  address_info.set_map_name_key(
      InternStringIfNecessaryAndGetKey(std::move(*address_info.mutable_map_name())));

  CaptureEvent event;
  *event.mutable_address_info() = std::move(address_info);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnTracepointEvent(orbit_grpc_protos::TracepointEvent tracepoint_event) {
  CHECK(tracepoint_event.tracepoint_info_or_key_case() ==
        orbit_grpc_protos::TracepointEvent::kTracepointInfo);
  tracepoint_event.set_tracepoint_info_key(
      InternTracepointInfoIfNecessaryAndGetKey(tracepoint_event.tracepoint_info()));

  CaptureEvent event;
  *event.mutable_tracepoint_event() = std::move(tracepoint_event);
  capture_event_buffer_->AddEvent(std::move(event));
}

void LinuxTracingHandler::OnModulesUpdate(
    orbit_grpc_protos::ModulesUpdateEvent modules_update_event) {
  orbit_grpc_protos::CaptureEvent event;
  *event.mutable_modules_update_event() = std::move(modules_update_event);

  capture_event_buffer_->AddEvent(std::move(event));
}

uint64_t LinuxTracingHandler::ComputeCallstackKey(const Callstack& callstack) {
  uint64_t key = 17;
  for (uint64_t pc : callstack.pcs()) {
    key = 31 * key + pc;
  }
  return key;
}

uint64_t LinuxTracingHandler::InternCallstackIfNecessaryAndGetKey(Callstack callstack) {
  uint64_t key = ComputeCallstackKey(callstack);
  {
    absl::MutexLock lock{&callstack_keys_sent_mutex_};
    if (callstack_keys_sent_.contains(key)) {
      return key;
    }
    callstack_keys_sent_.emplace(key);
  }

  CaptureEvent event;
  event.mutable_interned_callstack()->set_key(key);
  *event.mutable_interned_callstack()->mutable_intern() = std::move(callstack);
  capture_event_buffer_->AddEvent(std::move(event));
  return key;
}

uint64_t LinuxTracingHandler::ComputeStringKey(const std::string& str) {
  return std::hash<std::string>{}(str);
}

uint64_t LinuxTracingHandler::InternStringIfNecessaryAndGetKey(std::string str) {
  uint64_t key = ComputeStringKey(str);
  {
    absl::MutexLock lock{&string_keys_sent_mutex_};
    if (string_keys_sent_.contains(key)) {
      return key;
    }
    string_keys_sent_.emplace(key);
  }

  CaptureEvent event;
  event.mutable_interned_string()->set_key(key);
  event.mutable_interned_string()->set_intern(std::move(str));
  capture_event_buffer_->AddEvent(std::move(event));
  return key;
}

uint64_t LinuxTracingHandler::InternTracepointInfoIfNecessaryAndGetKey(
    const orbit_grpc_protos::TracepointInfo& tracepoint_info) {
  uint64_t key =
      ComputeStringKey(absl::StrCat(tracepoint_info.category(), ":", tracepoint_info.name()));
  {
    absl::MutexLock lock{&tracepoint_keys_sent_mutex_};
    if (tracepoint_keys_sent_.contains(key)) {
      return key;
    }
    tracepoint_keys_sent_.emplace(key);
  }

  CaptureEvent event;
  event.mutable_interned_tracepoint_info()->set_key(key);
  event.mutable_interned_tracepoint_info()->mutable_intern()->set_name(tracepoint_info.name());
  event.mutable_interned_tracepoint_info()->mutable_intern()->set_category(
      tracepoint_info.category());
  capture_event_buffer_->AddEvent(std::move(event));
  return key;
}

}  // namespace orbit_service
