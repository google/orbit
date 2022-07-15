// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracingHandler.h"

#include <absl/synchronization/mutex.h>

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;

using orbit_grpc_protos::kWindowsTracingProducerId;

void TracingHandler::Start(CaptureOptions capture_options) {
  ORBIT_CHECK(tracer_ == nullptr);
  tracer_ = orbit_windows_tracing::Tracer::Create(std::move(capture_options), this);
  tracer_->Start();
}

void TracingHandler::Stop() {
  ORBIT_CHECK(tracer_ != nullptr);
  tracer_->Stop();
  tracer_.reset();
  ORBIT_LOG("Windows TracingHandler stopped: ETW tracing is done");
}

void TracingHandler::OnSchedulingSlice(SchedulingSlice scheduling_slice) {
  ProducerCaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(scheduling_slice);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnCallstackSample(FullCallstackSample callstack_sample) {
  ProducerCaptureEvent event;
  *event.mutable_full_callstack_sample() = std::move(callstack_sample);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnFunctionCall(FunctionCall function_call) {
  ProducerCaptureEvent event;
  *event.mutable_function_call() = std::move(function_call);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnThreadNamesSnapshot(
    orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) {
  ProducerCaptureEvent event;
  *event.mutable_thread_names_snapshot() = std::move(thread_names_snapshot);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) {
  ProducerCaptureEvent event;
  *event.mutable_module_update_event() = std::move(module_update_event);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) {
  ProducerCaptureEvent event;
  *event.mutable_modules_snapshot() = std::move(modules_snapshot);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

void TracingHandler::OnPresentEvent(orbit_grpc_protos::PresentEvent present_event) {
  ProducerCaptureEvent event;
  *event.mutable_present_event() = std::move(present_event);
  producer_event_processor_->ProcessEvent(kWindowsTracingProducerId, std::move(event));
}

}  // namespace orbit_windows_capture_service
