// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxCaptureService/LinuxCaptureService.h"

#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_cat.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "ApiLoader/EnableInTracee.h"
#include "ApiUtils/Event.h"
#include "CaptureService/CommonProducerCaptureEventBuilders.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "Introspection/Introspection.h"
#include "MemoryInfoHandler.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/Profiling.h"
#include "OrbitVersion/OrbitVersion.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "TracingHandler.h"
#include "UserSpaceInstrumentationAddressesImpl.h"

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::GrpcClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

using orbit_capture_service::CaptureStartStopListener;

namespace orbit_linux_capture_service {

// Remove the functions with ids in `filter_function_ids` from instrumented_functions in
// `capture_options`.
static void FilterOutInstrumentedFunctionsFromCaptureOptions(
    const absl::flat_hash_set<uint64_t>& filter_function_ids, CaptureOptions& capture_options) {
  // Move the entries that need to be deleted to the end of the repeated field and remove them with
  // one single call to `DeleteSubrange`. This avoids quadratic complexity.
  int first_to_delete = 0;
  for (int i = 0; i < capture_options.instrumented_functions_size(); i++) {
    const uint64_t function_id = capture_options.instrumented_functions(i).function_id();
    if (!filter_function_ids.contains(function_id)) {
      first_to_delete++;
      if (i >= first_to_delete) {
        capture_options.mutable_instrumented_functions()->SwapElements(i, first_to_delete - 1);
      }
    }
  }
  capture_options.mutable_instrumented_functions()->DeleteSubrange(
      first_to_delete, capture_options.instrumented_functions_size() - first_to_delete);
}

[[nodiscard]] static std::unique_ptr<orbit_introspection::IntrospectionListener>
CreateIntrospectionListener(ProducerEventProcessor* producer_event_processor) {
  return std::make_unique<orbit_introspection::IntrospectionListener>(
      [producer_event_processor](const orbit_api::ApiEventVariant& api_event_variant) {
        ProducerCaptureEvent capture_event;
        std::visit(
            [&capture_event](const auto& api_event) {
              orbit_api::FillProducerCaptureEventFromApiEvent(api_event, &capture_event);
            },
            api_event_variant);
        producer_event_processor->ProcessEvent(orbit_grpc_protos::kIntrospectionProducerId,
                                               std::move(capture_event));
      });
}

// TracingHandler::Stop is blocking, until all perf_event_open events have been processed
// and all perf_event_open file descriptors have been closed.
// CaptureStartStopListener::OnCaptureStopRequested is also to be assumed blocking,
// for example until all CaptureEvents from external producers have been received.
// Hence why these methods need to be called in parallel on different threads.
static void StopInternalProducersAndCaptureStartStopListenersInParallel(
    TracingHandler* tracing_handler, MemoryInfoHandler* memory_info_handler,
    absl::flat_hash_set<CaptureStartStopListener*>* capture_start_stop_listeners) {
  std::vector<std::thread> stop_threads;

  stop_threads.emplace_back([&tracing_handler] {
    tracing_handler->Stop();
    LOG("TracingHandler stopped: perf_event_open tracing is done");
  });

  stop_threads.emplace_back([&memory_info_handler] {
    memory_info_handler->Stop();
    LOG("MemoryInfoHandler stopped: memory usage information collection is done");
  });

  for (CaptureStartStopListener* listener : *capture_start_stop_listeners) {
    stop_threads.emplace_back([&listener] {
      listener->OnCaptureStopRequested();
      LOG("CaptureStartStopListener stopped: one or more producers finished capturing");
    });
  }

  for (std::thread& stop_thread : stop_threads) {
    stop_thread.join();
  }
}

namespace {

// This class hijacks FunctionEntry and FunctionExit events before they reach the
// ProducerEventProcessor, and sends them to LinuxTracing instead, so that they can be processed
// like u(ret)probes. All the other events are forwarded to the ProducerEventProcessor normally.
class ProducerEventProcessorHijackingFunctionEntryExitForLinuxTracing
    : public ProducerEventProcessor {
 public:
  ProducerEventProcessorHijackingFunctionEntryExitForLinuxTracing(
      ProducerEventProcessor* original_producer_event_processor, TracingHandler* tracing_handler)
      : producer_event_processor_{original_producer_event_processor},
        tracing_handler_{tracing_handler} {}
  ~ProducerEventProcessorHijackingFunctionEntryExitForLinuxTracing() override = default;

  void ProcessEvent(uint64_t producer_id,
                    orbit_grpc_protos::ProducerCaptureEvent&& event) override {
    switch (event.event_case()) {
      case ProducerCaptureEvent::kFunctionEntry:
        tracing_handler_->ProcessFunctionEntry(event.function_entry());
        break;
      case ProducerCaptureEvent::kFunctionExit:
        tracing_handler_->ProcessFunctionExit(event.function_exit());
        break;
      case ProducerCaptureEvent::EVENT_NOT_SET:
        UNREACHABLE();
      default:
        producer_event_processor_->ProcessEvent(producer_id, std::move(event));
    }
  }

 private:
  ProducerEventProcessor* producer_event_processor_;
  TracingHandler* tracing_handler_;
};

}  // namespace

grpc::Status LinuxCaptureService::Capture(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("CSImpl::Capture");

  if (grpc::Status result = InitializeCapture(reader_writer); !result.ok()) {
    return result;
  }

  TracingHandler tracing_handler{producer_event_processor_.get()};
  ProducerEventProcessorHijackingFunctionEntryExitForLinuxTracing function_entry_exit_hijacker{
      producer_event_processor_.get(), &tracing_handler};
  MemoryInfoHandler memory_info_handler{producer_event_processor_.get()};

  CaptureRequest request = WaitForStartCaptureRequestFromClient(reader_writer);

  const CaptureOptions& capture_options = request.capture_options();

  // Enable Orbit API in tracee.
  std::optional<std::string> error_enabling_orbit_api;
  if (capture_options.enable_api()) {
    auto result = orbit_api_loader::EnableApiInTracee(capture_options);
    if (result.has_error()) {
      ERROR("Enabling Orbit Api: %s", result.error().message());
      error_enabling_orbit_api =
          absl::StrFormat("Could not enable Orbit API: %s", result.error().message());
    }
  }

  // We need to filter out the functions instrumented by user space instrumentation.
  CaptureOptions linux_tracing_capture_options;
  linux_tracing_capture_options.CopyFrom(capture_options);

  // Enable user space instrumentation.
  std::optional<std::string> error_enabling_user_space_instrumentation;
  std::optional<orbit_grpc_protos::ProducerCaptureEvent>
      info_from_enabling_user_space_instrumentation;
  std::unique_ptr<UserSpaceInstrumentationAddressesImpl> user_space_instrumentation_addresses;
  if (capture_options.dynamic_instrumentation_method() ==
          CaptureOptions::kUserSpaceInstrumentation &&
      capture_options.instrumented_functions_size() != 0) {
    auto result_or_error = instrumentation_manager_->InstrumentProcess(capture_options);
    if (result_or_error.has_error()) {
      error_enabling_user_space_instrumentation = absl::StrFormat(
          "Could not enable user space instrumentation: %s", result_or_error.error().message());
      ERROR("%s", error_enabling_user_space_instrumentation.value());
    } else {
      FilterOutInstrumentedFunctionsFromCaptureOptions(
          result_or_error.value().instrumented_function_ids, linux_tracing_capture_options);

      LOG("User space instrumentation enabled for %u out of %u instrumented functions.",
          result_or_error.value().instrumented_function_ids.size(),
          capture_options.instrumented_functions_size());

      if (!result_or_error.value().function_ids_to_error_messages.empty()) {
        info_from_enabling_user_space_instrumentation =
            orbit_capture_service::CreateWarningInstrumentingWithUserSpaceInstrumentationEvent(
                capture_start_timestamp_ns_,
                result_or_error.value().function_ids_to_error_messages);
      }

      user_space_instrumentation_addresses =
          std::make_unique<UserSpaceInstrumentationAddressesImpl>(
              result_or_error.value().entry_trampoline_address_ranges,
              result_or_error.value().return_trampoline_address_range,
              result_or_error.value().injected_library_path.string());
    }
  }

  StartEventProcessing(capture_options);

  if (error_enabling_orbit_api.has_value()) {
    producer_event_processor_->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        orbit_capture_service::CreateErrorEnablingOrbitApiEvent(
            capture_start_timestamp_ns_, std::move(error_enabling_orbit_api.value())));
  }

  if (error_enabling_user_space_instrumentation.has_value()) {
    producer_event_processor_->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        orbit_capture_service::CreateErrorEnablingUserSpaceInstrumentationEvent(
            capture_start_timestamp_ns_,
            std::move(error_enabling_user_space_instrumentation.value())));
  }

  if (info_from_enabling_user_space_instrumentation.has_value()) {
    producer_event_processor_->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        std::move(info_from_enabling_user_space_instrumentation.value()));
  }

  std::unique_ptr<orbit_introspection::IntrospectionListener> introspection_listener;
  if (capture_options.enable_introspection()) {
    introspection_listener = CreateIntrospectionListener(producer_event_processor_.get());
  }

  tracing_handler.Start(linux_tracing_capture_options,
                        std::move(user_space_instrumentation_addresses));

  memory_info_handler.Start(request.capture_options());
  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(request.capture_options(), &function_entry_exit_hijacker);
  }

  WaitForEndCaptureRequestFromClient(reader_writer);

  // Disable Orbit API in tracee.
  if (capture_options.enable_api()) {
    auto result = orbit_api_loader::DisableApiInTracee(capture_options);
    if (result.has_error()) {
      ERROR("Disabling Orbit Api: %s", result.error().message());
      producer_event_processor_->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          orbit_capture_service::CreateWarningEvent(
              orbit_base::CaptureTimestampNs(),
              absl::StrFormat("Could not disable Orbit API: %s", result.error().message())));
    }
  }

  // Disable user space instrumentation.
  if (capture_options.dynamic_instrumentation_method() ==
          CaptureOptions::kUserSpaceInstrumentation &&
      capture_options.instrumented_functions_size() != 0) {
    const pid_t target_process_id = orbit_base::ToNativeProcessId(capture_options.pid());
    auto result_tmp = instrumentation_manager_->UninstrumentProcess(target_process_id);
    if (result_tmp.has_error()) {
      ERROR("Disabling user space instrumentation: %s", result_tmp.error().message());
      producer_event_processor_->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          orbit_capture_service::CreateWarningEvent(
              orbit_base::CaptureTimestampNs(),
              absl::StrFormat("Could not disable user space instrumentation: %s",
                              result_tmp.error().message())));
    }
  }

  StopInternalProducersAndCaptureStartStopListenersInParallel(
      &tracing_handler, &memory_info_handler, &capture_start_stop_listeners_);

  // The destructor of IntrospectionListener takes care of actually disabling introspection.
  introspection_listener.reset();

  FinalizeEventProcessing();

  TerminateCapture();

  return grpc::Status::OK;
}

}  // namespace orbit_linux_capture_service
