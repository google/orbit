// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_CAPTURE_SERVICE_TRACING_HANDLER_H_
#define WINDOWS_CAPTURE_SERVICE_TRACING_HANDLER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "WindowsTracing/Tracer.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_capture_service {

// The TracingHandler is responsible for starting and stopping a trace as well as relaying the
// resulting events to an event processor.
class TracingHandler : public orbit_windows_tracing::TracerListener {
 public:
  explicit TracingHandler(
      orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor)
      : producer_event_processor_{producer_event_processor} {}

  ~TracingHandler() override = default;
  TracingHandler(const TracingHandler&) = delete;
  TracingHandler& operator=(const TracingHandler&) = delete;
  TracingHandler(TracingHandler&&) = delete;
  TracingHandler& operator=(TracingHandler&&) = delete;

  void Start(orbit_grpc_protos::CaptureOptions capture_options);
  void Stop();

  void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) override;
  void OnCallstackSample(orbit_grpc_protos::FullCallstackSample callstack_sample) override;
  void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) override;
  void OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) override;
  void OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) override;
  void OnThreadNamesSnapshot(orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) override;
  void OnPresentEvent(orbit_grpc_protos::PresentEvent present_event) override;

 private:
  orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor_;
  std::unique_ptr<orbit_windows_tracing::Tracer> tracer_;
};

}  // namespace orbit_windows_capture_service

#endif  // WINDOWS_CAPTURE_SERVICE_TRACING_HANDLER_H_
