// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxCaptureService/LinuxCaptureServiceBase.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "ApiLoader/EnableInTracee.h"
#include "ApiUtils/Event.h"
#include "CaptureServiceBase/CaptureStartStopListener.h"
#include "CaptureServiceBase/CommonProducerCaptureEventBuilders.h"
#include "CaptureServiceBase/StopCaptureRequestWaiter.h"
#include "ExtractSignalFromMinidump.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.pb.h"
#include "Introspection/Introspection.h"
#include "MemoryInfoHandler.h"
#include "MemoryWatchdog.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "TracingHandler.h"
#include "UserSpaceInstrumentationAddressesImpl.h"

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

using orbit_capture_service_base::CaptureServiceBase;
using orbit_capture_service_base::CaptureStartStopListener;
using orbit_capture_service_base::StopCaptureRequestWaiter;

namespace orbit_linux_capture_service {

namespace {

// Remove the functions with ids in `filter_function_ids` from instrumented_functions in
// `capture_options`.
void FilterOutInstrumentedFunctionsFromCaptureOptions(
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

[[nodiscard]] std::unique_ptr<orbit_introspection::IntrospectionListener>
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
void StopInternalProducersAndCaptureStartStopListenersInParallel(
    TracingHandler* tracing_handler, MemoryInfoHandler* memory_info_handler,
    absl::flat_hash_set<CaptureStartStopListener*>* capture_start_stop_listeners) {
  std::vector<std::thread> stop_threads;

  stop_threads.emplace_back([&tracing_handler] {
    tracing_handler->Stop();
    ORBIT_LOG("TracingHandler stopped: perf_event_open tracing is done");
  });

  stop_threads.emplace_back([&memory_info_handler] {
    memory_info_handler->Stop();
    ORBIT_LOG("MemoryInfoHandler stopped: memory usage information collection is done");
  });

  for (CaptureStartStopListener* listener : *capture_start_stop_listeners) {
    stop_threads.emplace_back([listener] {
      listener->OnCaptureStopRequested();
      ORBIT_LOG("CaptureStartStopListener stopped: one or more producers finished capturing");
    });
  }

  for (std::thread& stop_thread : stop_threads) {
    stop_thread.join();
  }
}

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

  void ProcessEvent(uint64_t producer_id, ProducerCaptureEvent&& event) override {
    switch (event.event_case()) {
      case ProducerCaptureEvent::kFunctionEntry:
        tracing_handler_->ProcessFunctionEntry(event.function_entry());
        break;
      case ProducerCaptureEvent::kFunctionExit:
        tracing_handler_->ProcessFunctionExit(event.function_exit());
        break;
      case ProducerCaptureEvent::EVENT_NOT_SET:
        ORBIT_UNREACHABLE();
      default:
        producer_event_processor_->ProcessEvent(producer_id, std::move(event));
    }
  }

 private:
  ProducerEventProcessor* producer_event_processor_;
  TracingHandler* tracing_handler_;
};

absl::flat_hash_set<std::string> ListFileNamesOfAllMinidumps() {
  absl::flat_hash_set<std::string> result;
  const std::string core_directory = "/usr/local/cloudcast/core";
  auto error_or_core_files = orbit_base::ListFilesInDirectory(core_directory);
  if (!error_or_core_files.has_error()) {
    const std::regex check_if_minidump_file(absl::StrFormat(R"(.*\.[0-9]+\.core\.dmp)"));
    for (const std::filesystem::path& path : error_or_core_files.value()) {
      if (regex_match(path.string(), check_if_minidump_file)) {
        result.insert(path.string());
      }
    }
  }
  return result;
}

struct TargetProcessStateAfterCapture {
  CaptureFinished::ProcessState process_state;
  CaptureFinished::TerminationSignal termination_signal;
};

TargetProcessStateAfterCapture GetTargetProcessStateAfterCapture(
    pid_t pid, const absl::flat_hash_set<std::string>& old_core_files) {
  TargetProcessStateAfterCapture result{
      .process_state = CaptureFinished::kProcessStateInternalError,
      .termination_signal = CaptureFinished::kTerminationSignalInternalError};

  const std::string pid_dir_name = absl::StrFormat("/proc/%i", pid);
  auto exists_or_error = orbit_base::FileOrDirectoryExists(pid_dir_name);
  if (exists_or_error.has_error()) {
    ORBIT_ERROR("Unable to check for existence of \"%s\": %s", pid_dir_name,
                exists_or_error.error().message());
    // We can't read the process state, so we return an error state.
    return result;
  }

  if (exists_or_error.value()) {
    // Process is still running.
    result.process_state = CaptureFinished::kRunning;
    result.termination_signal = CaptureFinished::kTerminationSignalUnspecified;
    return result;
  }

  // Check whether we find a minidump. Otherwise we assume the process ended gracefully.
  const std::string core_directory = "/usr/local/cloudcast/core";
  auto error_or_core_files = orbit_base::ListFilesInDirectory(core_directory);
  if (error_or_core_files.has_error()) {
    // We can't access the directory with the core files; we return an error state.
    return result;
  }
  const std::vector<std::filesystem::path>& core_files = error_or_core_files.value();
  // Matches zero or more characters ('.*'), followed by a literal dot ('\.'), followed by the pid
  // of the process ('%i'), followed by another literal dot ('\.'), followed by one or more numbers
  // ('[0-9]+') which is the number of seconds since the epoch finally, followed by the format
  // ending ('.core.dmp').
  const std::regex check_if_minidump_file(absl::StrFormat(R"(.*\.%i\.[0-9]+\.core\.dmp)", pid));
  for (const std::filesystem::path& path : core_files) {
    // Disregard the core file if it already existed at the start of the capture. We are only
    // interested in crashes from the current run. This protects against collisions; the pid of the
    // process might roll over and therefore not be unique.
    if (old_core_files.contains(path.string())) {
      continue;
    }
    if (regex_match(path.string(), check_if_minidump_file)) {
      result.process_state = CaptureFinished::kCrashed;
      ErrorMessageOr<int> signal_or_error = ExtractSignalFromMinidump(path);
      if (signal_or_error.has_error()) {
        ORBIT_ERROR("Error extracting termination signal from minidump: %s",
                    signal_or_error.error().message());
        return result;
      }
      const int signal = signal_or_error.value();
      result.termination_signal = static_cast<CaptureFinished::TerminationSignal>(signal);
      return result;
    }
  }

  // We did not find any core files for this process. So we assume a clean exit.
  result.process_state = CaptureFinished::kEnded;
  result.termination_signal = CaptureFinished::kTerminationSignalUnspecified;
  return result;
}

}  // namespace

CaptureServiceBase::StopCaptureReason
LinuxCaptureServiceBase::WaitForStopCaptureRequestOrMemoryThresholdExceeded(
    const std::shared_ptr<StopCaptureRequestWaiter>& stop_capture_request_waiter) {
  // wait_for_stop_capture_request_thread_ below outlives this method, hence the shared pointers.
  auto stop_capture_mutex = std::make_shared<absl::Mutex>();
  auto stop_capture = std::make_shared<bool>(false);
  auto stop_capture_reason = std::make_shared<std::optional<StopCaptureReason>>();

  wait_for_stop_capture_request_thread_ = std::thread{
      [stop_capture_request_waiter, stop_capture_mutex, stop_capture, stop_capture_reason] {
        // - For a GrpcStartStopCaptureRequestWaiter, this will wait on ServerReaderWriter::Read,
        //   which blocks until the client has called WritesDone, or until we finish the
        //   gRPC (before the client has called WritesDone). In the latter case, the Read unblocks
        //   *after* LinuxCaptureServiceBase::DoCapture has returned, so we need to keep the thread
        //   around and join it at a later time (we don't want to just detach it).
        // - For a CloudCollectorStartStopCaptureRequestWaiter, this will wait until
        //   CloudCollectorStartStopCaptureRequestWaiter::StopCapture is called externally.
        StopCaptureReason external_stop_reason =
            stop_capture_request_waiter->WaitForStopCaptureRequest();

        absl::MutexLock lock{stop_capture_mutex.get()};
        if (!*stop_capture) {
          ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");
          *stop_capture = true;
          *stop_capture_reason = external_stop_reason;
        } else {
          ORBIT_LOG(
              "Client finished writing on Capture's gRPC stream or the RPC has already finished; "
              "the capture was already stopped");
        }
      }};

  static const uint64_t mem_total_bytes = GetPhysicalMemoryInBytes();
  static const uint64_t watchdog_threshold_bytes = mem_total_bytes / 2;
  ORBIT_LOG("Starting memory watchdog with threshold %u B because total physical memory is %u B",
            watchdog_threshold_bytes, mem_total_bytes);
  while (true) {
    {
      absl::MutexLock lock{stop_capture_mutex.get()};
      static constexpr absl::Duration kWatchdogPollInterval = absl::Seconds(1);
      if (stop_capture_mutex->AwaitWithTimeout(absl::Condition(stop_capture.get()),
                                               kWatchdogPollInterval)) {
        ORBIT_LOG("Stopping memory watchdog as the capture was stopped");
        break;
      }
    }

    // Repeatedly poll the resident set size (rss) of the current process (OrbitService).
    std::optional<uint64_t> rss_bytes = ReadRssInBytesFromProcPidStat();
    if (!rss_bytes.has_value()) {
      ORBIT_ERROR_ONCE("Reading resident set size of OrbitService");
      continue;
    }
    if (rss_bytes.value() > watchdog_threshold_bytes) {
      ORBIT_LOG("Memory threshold exceeded: stopping capture (and stopping memory watchdog)");
      absl::MutexLock lock{stop_capture_mutex.get()};
      *stop_capture = true;
      *stop_capture_reason = StopCaptureReason::kMemoryWatchdog;
      break;
    }
  }

  // The memory watchdog loop exits when either the stop capture is requested, or the
  // memory threshold was exceeded. So at that point we can proceed with stopping the capture.
  {
    absl::MutexLock lock{stop_capture_mutex.get()};
    ORBIT_CHECK(stop_capture_reason->has_value());
    return stop_capture_reason->value();
  }
}

void LinuxCaptureServiceBase::DoCapture(
    const CaptureOptions& capture_options,
    const std::shared_ptr<StopCaptureRequestWaiter>& stop_capture_request_waiter) {
  if (wait_for_stop_capture_request_thread_.joinable()) {
    wait_for_stop_capture_request_thread_.join();
  }

  const absl::flat_hash_set<std::string> old_core_files = ListFileNamesOfAllMinidumps();

  TracingHandler tracing_handler{producer_event_processor_.get()};
  ProducerEventProcessorHijackingFunctionEntryExitForLinuxTracing function_entry_exit_hijacker{
      producer_event_processor_.get(), &tracing_handler};
  MemoryInfoHandler memory_info_handler{producer_event_processor_.get()};

  // Enable Orbit API in tracee.
  std::optional<std::string> error_enabling_orbit_api;
  if (capture_options.enable_api()) {
    auto result = orbit_api_loader::EnableApiInTracee(capture_options);
    if (result.has_error()) {
      ORBIT_ERROR("Enabling Orbit Api: %s", result.error().message());
      error_enabling_orbit_api =
          absl::StrFormat("Could not enable Orbit API: %s", result.error().message());
    }
  }

  // We need to filter out the functions instrumented by user space instrumentation.
  CaptureOptions linux_tracing_capture_options;
  linux_tracing_capture_options.CopyFrom(capture_options);

  // Enable user space instrumentation.
  std::optional<std::string> error_enabling_user_space_instrumentation;
  std::optional<ProducerCaptureEvent> info_from_enabling_user_space_instrumentation;
  std::unique_ptr<UserSpaceInstrumentationAddressesImpl> user_space_instrumentation_addresses;
  if (capture_options.dynamic_instrumentation_method() ==
          CaptureOptions::kUserSpaceInstrumentation &&
      capture_options.instrumented_functions_size() != 0) {
    auto result_or_error = instrumentation_manager_->InstrumentProcess(capture_options);
    if (result_or_error.has_error()) {
      error_enabling_user_space_instrumentation = absl::StrFormat(
          "Could not enable user space instrumentation: %s", result_or_error.error().message());
      ORBIT_ERROR("%s", error_enabling_user_space_instrumentation.value());
    } else {
      FilterOutInstrumentedFunctionsFromCaptureOptions(
          result_or_error.value().instrumented_function_ids, linux_tracing_capture_options);

      ORBIT_LOG("User space instrumentation enabled for %u out of %u instrumented functions.",
                result_or_error.value().instrumented_function_ids.size(),
                capture_options.instrumented_functions_size());

      if (!result_or_error.value().function_ids_to_error_messages.empty()) {
        info_from_enabling_user_space_instrumentation =
            orbit_capture_service_base::CreateWarningInstrumentingWithUserSpaceInstrumentationEvent(
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
        orbit_capture_service_base::CreateErrorEnablingOrbitApiEvent(
            capture_start_timestamp_ns_, std::move(error_enabling_orbit_api.value())));
  }

  if (error_enabling_user_space_instrumentation.has_value()) {
    producer_event_processor_->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        orbit_capture_service_base::CreateErrorEnablingUserSpaceInstrumentationEvent(
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

  memory_info_handler.Start(capture_options);
  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(capture_options, &function_entry_exit_hijacker);
  }

  StopCaptureReason stop_capture_reason =
      WaitForStopCaptureRequestOrMemoryThresholdExceeded(stop_capture_request_waiter);

  // Disable Orbit API in tracee.
  if (capture_options.enable_api()) {
    auto result = orbit_api_loader::DisableApiInTracee(capture_options);
    if (result.has_error()) {
      ORBIT_ERROR("Disabling Orbit Api: %s", result.error().message());
      producer_event_processor_->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          orbit_capture_service_base::CreateWarningEvent(
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
      ORBIT_ERROR("Disabling user space instrumentation: %s", result_tmp.error().message());
      producer_event_processor_->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          orbit_capture_service_base::CreateWarningEvent(
              orbit_base::CaptureTimestampNs(),
              absl::StrFormat("Could not disable user space instrumentation: %s",
                              result_tmp.error().message())));
    }
  }

  StopInternalProducersAndCaptureStartStopListenersInParallel(
      &tracing_handler, &memory_info_handler, &capture_start_stop_listeners_);

  // The destructor of IntrospectionListener takes care of actually disabling introspection.
  introspection_listener.reset();

  // Check whether the target process is still running and send that information.
  auto target_process_state = GetTargetProcessStateAfterCapture(
      orbit_base::ToNativeProcessId(capture_options.pid()), old_core_files);

  FinalizeEventProcessing(stop_capture_reason, target_process_state.process_state,
                          target_process_state.termination_signal);

  TerminateCapture();
}

}  // namespace orbit_linux_capture_service
