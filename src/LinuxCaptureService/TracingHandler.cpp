// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracingHandler.h"

#include <utility>

#include "GrpcProtos/Constants.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "OrbitBase/Logging.h"
#include "UserSpaceInstrumentationAddressesImpl.h"

namespace orbit_linux_capture_service {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FullGpuJob;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::ThreadStateSliceCallstack;

using orbit_grpc_protos::kLinuxTracingProducerId;

void TracingHandler::Start(
    const CaptureOptions& capture_options,
    std::unique_ptr<UserSpaceInstrumentationAddressesImpl> user_space_instrumentation_addresses) {
  ORBIT_CHECK(tracer_ == nullptr);

  tracer_ = orbit_linux_tracing::Tracer::Create(
      capture_options, std::move(user_space_instrumentation_addresses), this);
  tracer_->Start();
}

void TracingHandler::Stop() {
  ORBIT_CHECK(tracer_ != nullptr);
  tracer_->Stop();
  // tracer_ is not reset as FunctionEntry and FunctionExit events could still arrive afterwards. In
  // that case the Tracer will simply not process them.
  // Leaving the reset to the destructor means that an object of this class cannot be reused by
  // calling Start again.
}

void TracingHandler::OnSchedulingSlice(SchedulingSlice scheduling_slice) {
  ProducerCaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(scheduling_slice);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnThreadStateSliceCallstack(ThreadStateSliceCallstack callstack) {
  ProducerCaptureEvent event;
  *event.mutable_thread_state_slice_callstack() = std::move(callstack);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnCallstackSample(FullCallstackSample callstack_sample) {
  ProducerCaptureEvent event;
  *event.mutable_full_callstack_sample() = std::move(callstack_sample);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnFunctionCall(FunctionCall function_call) {
  ProducerCaptureEvent event;
  *event.mutable_function_call() = std::move(function_call);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnGpuJob(FullGpuJob full_gpu_job) {
  ProducerCaptureEvent event;
  *event.mutable_full_gpu_job() = std::move(full_gpu_job);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnThreadName(ThreadName thread_name) {
  ProducerCaptureEvent event;
  *event.mutable_thread_name() = std::move(thread_name);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnThreadNamesSnapshot(
    orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) {
  ProducerCaptureEvent event;
  *event.mutable_thread_names_snapshot() = std::move(thread_names_snapshot);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnThreadStateSlice(ThreadStateSlice thread_state_slice) {
  ProducerCaptureEvent event;
  *event.mutable_thread_state_slice() = std::move(thread_state_slice);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnAddressInfo(FullAddressInfo full_address_info) {
  ProducerCaptureEvent event;
  *event.mutable_full_address_info() = std::move(full_address_info);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnTracepointEvent(orbit_grpc_protos::FullTracepointEvent tracepoint_event) {
  ProducerCaptureEvent event;
  *event.mutable_full_tracepoint_event() = std::move(tracepoint_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) {
  ProducerCaptureEvent event;
  *event.mutable_module_update_event() = std::move(module_update_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) {
  ProducerCaptureEvent event;
  *event.mutable_modules_snapshot() = std::move(modules_snapshot);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnErrorsWithPerfEventOpenEvent(
    orbit_grpc_protos::ErrorsWithPerfEventOpenEvent errors_with_perf_event_open_event) {
  ProducerCaptureEvent event;
  *event.mutable_errors_with_perf_event_open_event() = std::move(errors_with_perf_event_open_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnLostPerfRecordsEvent(
    orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event) {
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_lost_perf_records_event() = std::move(lost_perf_records_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnOutOfOrderEventsDiscardedEvent(
    orbit_grpc_protos::OutOfOrderEventsDiscardedEvent out_of_order_events_discarded_event) {
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_out_of_order_events_discarded_event() =
      std::move(out_of_order_events_discarded_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

void TracingHandler::OnWarningInstrumentingWithUprobesEvent(
    orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
        warning_instrumenting_with_uprobes_event) {
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_warning_instrumenting_with_uprobes_event() =
      std::move(warning_instrumenting_with_uprobes_event);
  producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(event));
}

}  // namespace orbit_linux_capture_service
