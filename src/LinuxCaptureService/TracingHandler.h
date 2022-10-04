// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_LINUX_TRACING_HANDLER_H_
#define LINUX_CAPTURE_SERVICE_LINUX_TRACING_HANDLER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "Introspection/Introspection.h"
#include "LinuxTracing/Tracer.h"
#include "LinuxTracing/TracerListener.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "UserSpaceInstrumentationAddressesImpl.h"

namespace orbit_linux_capture_service {

// Wrapper around LinuxTracing and its orbit_linux_tracing::Tracer that forwards the received events
// to the ProducerEventProcessor.
// An instance of this class should not be reused for multiple captures, i.e., Start and Stop should
// only be called once.
class TracingHandler : public orbit_linux_tracing::TracerListener {
 public:
  explicit TracingHandler(
      orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor)
      : producer_event_processor_{producer_event_processor} {}

  ~TracingHandler() override = default;
  TracingHandler(const TracingHandler&) = delete;
  TracingHandler& operator=(const TracingHandler&) = delete;
  TracingHandler(TracingHandler&&) = delete;
  TracingHandler& operator=(TracingHandler&&) = delete;

  void Start(
      const orbit_grpc_protos::CaptureOptions& capture_options,
      std::unique_ptr<UserSpaceInstrumentationAddressesImpl> user_space_instrumentation_addresses);
  void Stop();

  void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) override;
  void OnCallstackSample(orbit_grpc_protos::FullCallstackSample callstack_sample) override;
  void OnThreadStateSliceCallstack(orbit_grpc_protos::ThreadStateSliceCallstack callstack) override;
  void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) override;
  void OnGpuJob(orbit_grpc_protos::FullGpuJob gpu_job) override;
  void OnThreadName(orbit_grpc_protos::ThreadName thread_name) override;
  void OnThreadNamesSnapshot(orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) override;
  void OnThreadStateSlice(orbit_grpc_protos::ThreadStateSlice thread_state_slice) override;
  void OnAddressInfo(orbit_grpc_protos::FullAddressInfo full_address_info) override;
  void OnTracepointEvent(orbit_grpc_protos::FullTracepointEvent tracepoint_event) override;
  void OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) override;
  void OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) override;
  void OnErrorsWithPerfEventOpenEvent(
      orbit_grpc_protos::ErrorsWithPerfEventOpenEvent errors_with_perf_event_open_event) override;
  void OnLostPerfRecordsEvent(
      orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event) override;
  void OnOutOfOrderEventsDiscardedEvent(orbit_grpc_protos::OutOfOrderEventsDiscardedEvent
                                            out_of_order_events_discarded_event) override;
  void OnWarningInstrumentingWithUprobesEvent(
      orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
          warning_instrumenting_with_uprobes_event) override;
  void OnApiScopeStart(orbit_grpc_protos::ApiScopeStart api_scope_start) override;
  void OnApiScopeStartAsync(orbit_grpc_protos::ApiScopeStartAsync api_scope_start_async) override;
  void OnApiScopeStop(orbit_grpc_protos::ApiScopeStop api_scope_stop) override;
  void OnApiScopeStopAsync(orbit_grpc_protos::ApiScopeStopAsync api_scope_stop_async) override;
  void OnApiStringEvent(orbit_grpc_protos::ApiStringEvent api_string_event) override;
  void OnApiTrackDouble(orbit_grpc_protos::ApiTrackDouble api_track_double) override;
  void OnApiTrackFloat(orbit_grpc_protos::ApiTrackFloat api_track_float) override;
  void OnApiTrackInt(orbit_grpc_protos::ApiTrackInt api_track_int) override;
  void OnApiTrackInt64(orbit_grpc_protos::ApiTrackInt64 api_track_int64) override;
  void OnApiTrackUint(orbit_grpc_protos::ApiTrackUint api_track_uint) override;
  void OnApiTrackUint64(orbit_grpc_protos::ApiTrackUint64 api_track_uint64) override;

  // The Proceess* methods are used to pipe back grpc protos from external producers to linux
  // tracing. Compare code in ProducerEventProcessorHijackingForLinuxTracing.
  void ProcessApiScopeStart(const orbit_grpc_protos::ApiScopeStart& api_scope_start) {
    tracer_->ProcessApiScopeStart(api_scope_start);
  }
  void ProcessApiScopeStartAsync(
      const orbit_grpc_protos::ApiScopeStartAsync& api_scope_start_async) {
    tracer_->ProcessApiScopeStartAsync(api_scope_start_async);
  }
  void ProcessApiScopeStop(const orbit_grpc_protos::ApiScopeStop& api_scope_stop) {
    tracer_->ProcessApiScopeStop(api_scope_stop);
  }
  void ProcessApiScopeStopAsync(const orbit_grpc_protos::ApiScopeStopAsync& api_scope_stop_async) {
    tracer_->ProcessApiScopeStopAsync(api_scope_stop_async);
  }
  void ProcessApiStringEvent(const orbit_grpc_protos::ApiStringEvent& api_string_event) {
    tracer_->ProcessApiStringEvent(api_string_event);
  }
  void ProcessApiTrackDouble(const orbit_grpc_protos::ApiTrackDouble& api_track_double) {
    tracer_->ProcessApiTrackDouble(api_track_double);
  }
  void ProcessApiTrackFloat(const orbit_grpc_protos::ApiTrackFloat& api_track_float) {
    tracer_->ProcessApiTrackFloat(api_track_float);
  }
  void ProcessApiTrackInt(const orbit_grpc_protos::ApiTrackInt& api_track_int) {
    tracer_->ProcessApiTrackInt(api_track_int);
  }
  void ProcessApiTrackInt64(const orbit_grpc_protos::ApiTrackInt64& api_track_int64) {
    tracer_->ProcessApiTrackInt64(api_track_int64);
  }
  void ProcessApiTrackUint(const orbit_grpc_protos::ApiTrackUint& api_track_uint) {
    tracer_->ProcessApiTrackUint(api_track_uint);
  }
  void ProcessApiTrackUint64(const orbit_grpc_protos::ApiTrackUint64& api_track_uint64) {
    tracer_->ProcessApiTrackUint64(api_track_uint64);
  }
  void ProcessFunctionEntry(const orbit_grpc_protos::FunctionEntry& function_entry) {
    tracer_->ProcessFunctionEntry(function_entry);
  }
  void ProcessFunctionExit(const orbit_grpc_protos::FunctionExit& function_exit) {
    tracer_->ProcessFunctionExit(function_exit);
  }

 private:
  orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor_;
  std::unique_ptr<orbit_linux_tracing::Tracer> tracer_;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_LINUX_TRACING_HANDLER_H_
