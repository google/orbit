// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceImpl.h"

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <pthread.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "ApiLoader/EnableInTracee.h"
#include "ApiUtils/Event.h"
#include "CaptureEventBuffer.h"
#include "CaptureEventSender.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "MemoryInfoHandler.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitVersion/OrbitVersion.h"
#include "ProducerEventProcessor.h"
#include "TracingHandler.h"
#include "capture.pb.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

namespace {

using orbit_grpc_protos::ClientCaptureEvent;

class SenderThreadCaptureEventBuffer final : public CaptureEventBuffer {
 public:
  explicit SenderThreadCaptureEventBuffer(CaptureEventSender* event_sender)
      : capture_event_sender_{event_sender} {
    CHECK(capture_event_sender_ != nullptr);
    sender_thread_ = std::thread{[this] { SenderThread(); }};
  }

  void AddEvent(ClientCaptureEvent&& event) override {
    absl::MutexLock lock{&events_being_buffered_mutex_};
    if (stop_requested_) {
      return;
    }
    events_being_buffered_.emplace_back(std::move(event));
  }

  void StopAndWait() {
    CHECK(sender_thread_.joinable());
    {
      // Protect stop_requested_ with event_buffer_mutex_ so that we can use stop_requested_
      // in Conditions for Await/LockWhen (specifically, in SenderThread).
      absl::MutexLock lock{&events_being_buffered_mutex_};
      stop_requested_ = true;
    }
    sender_thread_.join();
  }

  ~SenderThreadCaptureEventBuffer() override { CHECK(!sender_thread_.joinable()); }

 private:
  void SenderThread() {
    orbit_base::SetCurrentThreadName("SenderThread");
    constexpr absl::Duration kSendTimeInterval = absl::Milliseconds(20);
    // This should be lower than kMaxEventsPerResponse in GrpcCaptureEventSender::SendEvents
    // as a few more events are likely to arrive after the condition becomes true.
    constexpr uint64_t kSendEventCountInterval = 5000;

    bool stopped = false;
    while (!stopped) {
      ORBIT_SCOPE("SenderThread iteration");

      events_being_buffered_mutex_.LockWhenWithTimeout(
          absl::Condition(
              +[](SenderThreadCaptureEventBuffer* self)
                   ABSL_EXCLUSIVE_LOCKS_REQUIRED(self->events_being_buffered_mutex_) {
                     return self->events_being_buffered_.size() >= kSendEventCountInterval ||
                            self->stop_requested_;
                   },
              this),
          kSendTimeInterval);
      if (stop_requested_) {
        stopped = true;
      }
      events_being_buffered_.swap(events_to_send_);
      events_being_buffered_mutex_.Unlock();

      capture_event_sender_->SendEvents(&events_to_send_);
      // std::vector::clear() "Leaves the capacity() of the vector unchanged", which is desired.
      events_to_send_.clear();
    }
  }

  std::vector<ClientCaptureEvent> events_being_buffered_
      ABSL_GUARDED_BY(events_being_buffered_mutex_);
  absl::Mutex events_being_buffered_mutex_;
  std::vector<ClientCaptureEvent> events_to_send_;
  CaptureEventSender* capture_event_sender_;
  std::thread sender_thread_;
  bool stop_requested_ ABSL_GUARDED_BY(events_being_buffered_mutex_) = false;
};

class GrpcCaptureEventSender final : public CaptureEventSender {
 public:
  explicit GrpcCaptureEventSender(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {
    CHECK(reader_writer_ != nullptr);
  }

  ~GrpcCaptureEventSender() override {
    LOG("Total number of events sent: %lu", total_number_of_events_sent_);
    LOG("Total number of bytes sent: %lu", total_number_of_bytes_sent_);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    float average_bytes =
        static_cast<float>(total_number_of_bytes_sent_) / total_number_of_events_sent_;

    LOG("Average number of bytes per event: %.2f", average_bytes);
  }

  void SendEvents(std::vector<ClientCaptureEvent>* events) override {
    ORBIT_SCOPE_FUNCTION;
    CHECK(events != nullptr);
    ORBIT_UINT64("Number of buffered events sent", events->size());
    if (events->empty()) {
      return;
    }

    constexpr uint64_t kMaxEventsPerResponse = 10'000;
    uint64_t number_of_bytes_sent = 0;
    CaptureResponse response;
    for (ClientCaptureEvent& event : *events) {
      // We buffer to avoid sending countless tiny messages, but we also want to
      // avoid huge messages, which would cause the capture on the client to jump
      // forward in time in few big steps and not look live anymore.
      if (response.capture_events_size() == kMaxEventsPerResponse) {
        number_of_bytes_sent += response.ByteSizeLong();
        reader_writer_->Write(response);
        response.clear_capture_events();
      }
      response.mutable_capture_events()->Add(std::move(event));
    }
    number_of_bytes_sent += response.ByteSizeLong();
    reader_writer_->Write(response);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    float average_bytes = static_cast<float>(number_of_bytes_sent) / events->size();

    ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);
    total_number_of_events_sent_ += events->size();
    total_number_of_bytes_sent_ += number_of_bytes_sent;
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;

  uint64_t total_number_of_events_sent_ = 0;
  uint64_t total_number_of_bytes_sent_ = 0;
};

}  // namespace

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

[[nodiscard]] static ProducerCaptureEvent CreateCaptureStartedEvent(
    const CaptureOptions& capture_options, absl::Time capture_start_time,
    uint64_t capture_start_timestamp_ns) {
  ProducerCaptureEvent event;
  CaptureStarted* capture_started = event.mutable_capture_started();

  pid_t target_pid = orbit_base::ToNativeProcessId(capture_options.pid());

  capture_started->set_process_id(target_pid);
  auto executable_path_or_error = orbit_base::GetExecutablePath(target_pid);

  if (executable_path_or_error.has_value()) {
    const std::string& executable_path = executable_path_or_error.value();
    capture_started->set_executable_path(executable_path);

    ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> elf_file_or_error =
        orbit_object_utils::CreateElfFile(executable_path);
    if (elf_file_or_error.has_value()) {
      capture_started->set_executable_build_id(elf_file_or_error.value()->GetBuildId());
    } else {
      ERROR("Unable to load module: %s", elf_file_or_error.error().message());
    }
  } else {
    ERROR("%s", executable_path_or_error.error().message());
  }

  capture_started->set_capture_start_unix_time_ns(absl::ToUnixNanos(capture_start_time));
  capture_started->set_capture_start_timestamp_ns(capture_start_timestamp_ns);
  orbit_version::Version version = orbit_version::GetVersion();
  capture_started->set_orbit_version_major(version.major_version);
  capture_started->set_orbit_version_minor(version.minor_version);
  capture_started->mutable_capture_options()->CopyFrom(capture_options);
  return event;
}

[[nodiscard]] static ProducerCaptureEvent CreateClockResolutionEvent(uint64_t timestamp_ns,
                                                                     uint64_t resolution_ns) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ClockResolutionEvent* clock_resolution_event =
      event.mutable_clock_resolution_event();
  clock_resolution_event->set_timestamp_ns(timestamp_ns);
  clock_resolution_event->set_clock_resolution_ns(resolution_ns);
  return event;
}

[[nodiscard]] static ProducerCaptureEvent CreateErrorEnablingOrbitApiEvent(uint64_t timestamp_ns,
                                                                           std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ErrorEnablingOrbitApiEvent* error_enabling_orbit_api_event =
      event.mutable_error_enabling_orbit_api_event();
  error_enabling_orbit_api_event->set_timestamp_ns(timestamp_ns);
  error_enabling_orbit_api_event->set_message(std::move(message));
  return event;
}

[[nodiscard]] static ProducerCaptureEvent CreateErrorEnablingUserSpaceInstrumentationEvent(
    uint64_t timestamp_ns, std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent*
      error_enabling_user_space_instrumentation_event =
          event.mutable_error_enabling_user_space_instrumentation_event();
  error_enabling_user_space_instrumentation_event->set_timestamp_ns(timestamp_ns);
  error_enabling_user_space_instrumentation_event->set_message(std::move(message));
  return event;
}

[[nodiscard]] static ProducerCaptureEvent CreateWarningEvent(uint64_t timestamp_ns,
                                                             std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::WarningEvent* warning_event = event.mutable_warning_event();
  warning_event->set_timestamp_ns(timestamp_ns);
  warning_event->set_message(std::move(message));
  return event;
}

[[nodiscard]] static ClientCaptureEvent CreateCaptureFinishedEvent() {
  ClientCaptureEvent event;
  CaptureFinished* capture_finished = event.mutable_capture_finished();
  capture_finished->set_status(CaptureFinished::kSuccessful);
  return event;
}

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("CSImpl::Capture");
  if (is_capturing) {
    ERROR("Cannot start capture because another capture is already in progress");
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "Cannot start capture because another capture is already in progress.");
  }
  is_capturing = true;

  GrpcCaptureEventSender capture_event_sender{reader_writer};
  SenderThreadCaptureEventBuffer capture_event_buffer{&capture_event_sender};
  std::unique_ptr<ProducerEventProcessor> producer_event_processor =
      ProducerEventProcessor::Create(&capture_event_buffer);
  TracingHandler tracing_handler{producer_event_processor.get()};
  MemoryInfoHandler memory_info_handler{producer_event_processor.get()};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");

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
  if (capture_options.enable_user_space_instrumentation()) {
    auto result = instrumentation_manager_->InstrumentProcess(capture_options);
    if (result.has_error()) {
      error_enabling_user_space_instrumentation = absl::StrFormat(
          "Could not enable user space instrumentation: %s", result.error().message());
      ERROR("%s", error_enabling_user_space_instrumentation.value());
    } else {
      FilterOutInstrumentedFunctionsFromCaptureOptions(result.value(),
                                                       linux_tracing_capture_options);
      LOG("User space instrumentation enabled for %u out of %u instrumented functions.",
          result.value().size(), capture_options.instrumented_functions_size());
    }
  }

  // These are not in precise sync but they do not have to be.
  absl::Time capture_start_time = absl::Now();
  uint64_t capture_start_timestamp_ns = orbit_base::CaptureTimestampNs();

  producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      CreateCaptureStartedEvent(capture_options, capture_start_time, capture_start_timestamp_ns));

  producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      CreateClockResolutionEvent(capture_start_timestamp_ns, clock_resolution_ns_));

  if (error_enabling_orbit_api.has_value()) {
    producer_event_processor->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        CreateErrorEnablingOrbitApiEvent(capture_start_timestamp_ns,
                                         std::move(error_enabling_orbit_api.value())));
  }

  if (error_enabling_user_space_instrumentation.has_value()) {
    producer_event_processor->ProcessEvent(
        orbit_grpc_protos::kRootProducerId,
        CreateErrorEnablingUserSpaceInstrumentationEvent(
            capture_start_timestamp_ns,
            std::move(error_enabling_user_space_instrumentation.value())));
  }

  std::unique_ptr<orbit_introspection::IntrospectionListener> introspection_listener;
  if (capture_options.enable_introspection()) {
    introspection_listener = CreateIntrospectionListener(producer_event_processor.get());
  }

  tracing_handler.Start(linux_tracing_capture_options);

  memory_info_handler.Start(request.capture_options());
  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(request.capture_options(), producer_event_processor.get());
  }

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");

  // Disable Orbit API in tracee.
  if (capture_options.enable_api()) {
    auto result = orbit_api_loader::DisableApiInTracee(capture_options);
    if (result.has_error()) {
      ERROR("Disabling Orbit Api: %s", result.error().message());
      producer_event_processor->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          CreateWarningEvent(
              orbit_base::CaptureTimestampNs(),
              absl::StrFormat("Could not disable Orbit API: %s", result.error().message())));
    }
  }

  // Disable user space instrumentation.
  if (capture_options.enable_user_space_instrumentation()) {
    const pid_t target_process_id = orbit_base::ToNativeProcessId(capture_options.pid());
    auto result_tmp = instrumentation_manager_->UninstrumentProcess(target_process_id);
    if (result_tmp.has_error()) {
      ERROR("Disabling user space instrumentation: %s", result_tmp.error().message());
      producer_event_processor->ProcessEvent(
          orbit_grpc_protos::kRootProducerId,
          CreateWarningEvent(orbit_base::CaptureTimestampNs(),
                             absl::StrFormat("Could not disable user space instrumentation: %s",
                                             result_tmp.error().message())));
    }
  }

  StopInternalProducersAndCaptureStartStopListenersInParallel(
      &tracing_handler, &memory_info_handler, &capture_start_stop_listeners_);

  // The destructor of IntrospectionListener takes care of actually disabling introspection.
  introspection_listener.reset();

  capture_event_buffer.AddEvent(CreateCaptureFinishedEvent());

  capture_event_buffer.StopAndWait();
  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  is_capturing = false;
  return grpc::Status::OK;
}

void CaptureServiceImpl::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  CHECK(new_insertion);
}

void CaptureServiceImpl::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  CHECK(was_removed);
}

void CaptureServiceImpl::EstimateAndLogClockResolution() {
  // We expect the value to be small, ~35 nanoseconds.
  clock_resolution_ns_ = orbit_base::EstimateClockResolution();
  if (clock_resolution_ns_ > 0) {
    LOG("Clock resolution: %d (ns)", clock_resolution_ns_);
  } else {
    ERROR("Failed to estimate clock resolution");
  }
}

}  // namespace orbit_service
