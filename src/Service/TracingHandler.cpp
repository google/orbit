// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracingHandler.h"

#include <absl/synchronization/mutex.h>

#include <utility>

#include "ApiUtils/Event.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FullGpuJob;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;

using orbit_grpc_protos::kLinuxTracingProducerId;

void TracingHandler::Start(CaptureOptions capture_options) {
  CHECK(tracer_ == nullptr);
  bool enable_introspection = capture_options.enable_introspection();

  tracer_ = std::make_unique<orbit_linux_tracing::Tracer>(std::move(capture_options));
  tracer_->SetListener(this);
  tracer_->Start();

  if (enable_introspection) {
    SetupIntrospection();
  }
}

namespace {
inline void CreateCaptureEvent(const orbit_api::ApiScopeStart& scope_start,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start();
  scope_start.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiScopeStop& scope_stop,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop();
  scope_stop.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiScopeStartAsync& scope_start_async,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start_async();
  scope_start_async.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiScopeStopAsync& scope_stop_async,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop_async();
  scope_stop_async.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiStringEvent& string_event,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_string_event();
  string_event.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiTrackDouble& track_double,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_double();
  track_double.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiTrackFloat& track_float,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_float();
  track_float.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiTrackInt& track_int,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int();
  track_int.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiTrackInt64& track_int64,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int64();
  track_int64.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const orbit_api::ApiTrackUint& track_uint,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint();
  track_uint.CopyToGrpcProto(api_event);
}
inline void CreateCaptureEvent(const orbit_api::ApiTrackUint64& track_uint64,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint64();
  track_uint64.CopyToGrpcProto(api_event);
}

// The variant type `ApiEventVariant` requires to contain `std::monostate` in order to be default-
// constructable. However, that state is never expected to be called in the visitor.
inline void CreateCaptureEvent(const std::monostate& /*unused*/, ProducerCaptureEvent* /*unused*/) {
  UNREACHABLE();
}
}  // namespace

void TracingHandler::SetupIntrospection() {
  orbit_tracing_listener_ = std::make_unique<orbit_introspection::TracingListener>(
      [this](const orbit_api::ApiEventVariant& api_event_variant) {
        ProducerCaptureEvent capture_event;
        std::visit([&capture_event](
                       const auto& api_event) { CreateCaptureEvent(api_event, &capture_event); },
                   api_event_variant);
        producer_event_processor_->ProcessEvent(kLinuxTracingProducerId, std::move(capture_event));
      });
}

void TracingHandler::Stop() {
  CHECK(tracer_ != nullptr);
  tracer_->Stop();
  tracer_.reset();
}

void TracingHandler::OnSchedulingSlice(SchedulingSlice scheduling_slice) {
  ProducerCaptureEvent event;
  *event.mutable_scheduling_slice() = std::move(scheduling_slice);
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

}  // namespace orbit_service
