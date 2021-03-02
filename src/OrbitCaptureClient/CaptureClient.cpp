// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureClient.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/time/time.h>

#include <cstdint>
#include <outcome.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"
#include "capture.pb.h"
#include "tracepoint.pb.h"

ABSL_DECLARE_FLAG(uint16_t, sampling_rate);
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);

using orbit_client_protos::FunctionInfo;

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::TracepointInfo;

using orbit_base::Future;

static CaptureOptions::InstrumentedFunction::FunctionType InstrumentedFunctionTypeFromOrbitType(
    FunctionInfo::OrbitType orbit_type) {
  switch (orbit_type) {
    case FunctionInfo::kOrbitTimerStart:
      return CaptureOptions::InstrumentedFunction::kTimerStart;
    case FunctionInfo::kOrbitTimerStop:
      return CaptureOptions::InstrumentedFunction::kTimerStop;
    default:
      return CaptureOptions::InstrumentedFunction::kRegular;
  }
}

Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> CaptureClient::Capture(
    ThreadPool* thread_pool, const ProcessData& process,
    const orbit_client_data::ModuleManager& module_manager,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
    TracepointInfoSet selected_tracepoints, absl::flat_hash_set<uint64_t> frame_track_function_ids,
    bool collect_thread_state, bool bulk_capture_events, bool enable_introspection,
    uint64_t max_local_marker_depth_per_command_buffer) {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStopped) {
    return {
        ErrorMessage("Capture cannot be started, the previous capture is still "
                     "running/stopping.")};
  }

  state_ = State::kStarting;

  // TODO(168797897) Here a copy of the process is created. The loaded modules of this process were
  // likely filled when the process was selected, which might be a while back. Between then and now
  // the loaded modules might have changed. Up to date information about which modules are loaded
  // should be used here. (Even better: while taking a capture this should always be up to date)
  ProcessData process_copy = process;

  auto capture_result = thread_pool->Schedule(
      [this, process = std::move(process_copy), &module_manager,
       selected_functions = std::move(selected_functions), selected_tracepoints,
       frame_track_function_ids = std::move(frame_track_function_ids), collect_thread_state,
       bulk_capture_events, enable_introspection,
       max_local_marker_depth_per_command_buffer]() mutable {
        return CaptureSync(std::move(process), module_manager, std::move(selected_functions),
                           std::move(selected_tracepoints), std::move(frame_track_function_ids),
                           collect_thread_state, bulk_capture_events, enable_introspection,
                           max_local_marker_depth_per_command_buffer);
      });

  return capture_result;
}

ErrorMessageOr<CaptureListener::CaptureOutcome> CaptureClient::CaptureSync(
    ProcessData&& process, const orbit_client_data::ModuleManager& module_manager,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
    TracepointInfoSet selected_tracepoints, absl::flat_hash_set<uint64_t> frame_track_function_ids,
    bool collect_thread_state, bool bulk_capture_events, bool enable_introspection,
    uint64_t max_local_marker_depth_per_command_buffer) {
  ORBIT_SCOPE_FUNCTION;
  writes_done_failed_ = false;
  try_abort_ = false;
  {
    absl::WriterMutexLock lock{&context_and_stream_mutex_};
    CHECK(client_context_ == nullptr);
    CHECK(reader_writer_ == nullptr);
    client_context_ = std::make_unique<grpc::ClientContext>();
    reader_writer_ = capture_service_->Capture(client_context_.get());
  }

  CaptureRequest request;
  CaptureOptions* capture_options = request.mutable_capture_options();
  capture_options->set_trace_context_switches(true);
  capture_options->set_pid(process.pid());
  uint16_t sampling_rate = absl::GetFlag(FLAGS_sampling_rate);
  if (sampling_rate == 0) {
    capture_options->set_unwinding_method(CaptureOptions::kUndefined);
  } else {
    capture_options->set_sampling_rate(sampling_rate);
    if (absl::GetFlag(FLAGS_frame_pointer_unwinding)) {
      capture_options->set_unwinding_method(CaptureOptions::kFramePointers);
    } else {
      capture_options->set_unwinding_method(CaptureOptions::kDwarf);
    }
  }

  capture_options->set_trace_thread_state(collect_thread_state);
  capture_options->set_bulk_capture_events(bulk_capture_events);
  capture_options->set_trace_gpu_driver(true);
  capture_options->set_max_local_marker_depth_per_command_buffer(
      max_local_marker_depth_per_command_buffer);
  for (const auto& [function_id, function] : selected_functions) {
    CaptureOptions::InstrumentedFunction* instrumented_function =
        capture_options->add_instrumented_functions();
    instrumented_function->set_file_path(function.loaded_module_path());
    const ModuleData* module = module_manager.GetModuleByPath(function.loaded_module_path());
    CHECK(module != nullptr);
    instrumented_function->set_file_offset(function_utils::Offset(function, *module));
    instrumented_function->set_function_id(function_id);
    instrumented_function->set_function_type(
        InstrumentedFunctionTypeFromOrbitType(function.orbit_type()));
  }

  for (const auto& tracepoint : selected_tracepoints) {
    TracepointInfo* instrumented_tracepoint = capture_options->add_instrumented_tracepoint();
    instrumented_tracepoint->set_category(tracepoint.category());
    instrumented_tracepoint->set_name(tracepoint.name());
  }

  capture_options->set_enable_introspection(enable_introspection);

  bool request_write_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    request_write_succeeded = reader_writer_->Write(request);
    if (!request_write_succeeded) {
      reader_writer_->WritesDone();
    }
  }
  if (!request_write_succeeded) {
    ERROR("Sending CaptureRequest on Capture's gRPC stream");
    ErrorMessageOr<void> finish_result = FinishCapture();
    std::string error_string =
        absl::StrFormat("Error sending capture request.%s",
                        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }
  LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start capturing");

  {
    absl::MutexLock lock{&state_mutex_};
    state_ = State::kStarted;
  }

  CaptureEventProcessor event_processor(capture_listener_);

  capture_listener_->OnCaptureStarted(std::move(process), std::move(selected_functions),
                                      std::move(selected_tracepoints),
                                      std::move(frame_track_function_ids));

  while (!writes_done_failed_ && !try_abort_) {
    CaptureResponse response;
    bool read_succeeded;
    {
      absl::ReaderMutexLock lock{&context_and_stream_mutex_};
      read_succeeded = reader_writer_->Read(&response);
    }
    if (read_succeeded) {
      event_processor.ProcessEvents(response.capture_events());
    } else {
      break;
    }
  }

  ErrorMessageOr<void> finish_result = FinishCapture();
  if (try_abort_) {
    LOG("TryCancel on Capture's gRPC context was called: Read on Capture's gRPC stream failed");
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  if (writes_done_failed_) {
    LOG("WritesDone on Capture's gRPC stream failed: stop reading and try to finish the gRPC call");
    std::string error_string = absl::StrFormat(
        "Unable to finish the capture in orderly manner, performing emergency stop.%s",
        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }

  LOG("Finished reading from Capture's gRPC stream: all capture data has been received");
  if (finish_result.has_error()) {
    return ErrorMessage{absl::StrFormat(
        "Unable to finish the capture in an orderly manner. The following error occurred: %s",
        finish_result.error().message())};
  }
  return CaptureListener::CaptureOutcome::kComplete;
}

bool CaptureClient::StopCapture() {
  {
    absl::MutexLock lock(&state_mutex_);
    if (state_ == State::kStarting) {
      // Block and wait until the state is not kStarting
      bool (*is_not_starting)(State*) = [](State* state) { return *state != State::kStarting; };
      state_mutex_.Await(absl::Condition(is_not_starting, &state_));
    }

    if (state_ != State::kStarted) {
      LOG("StopCapture ignored, because it is already stopping or stopped");
      return false;
    }
    state_ = State::kStopping;
  }

  bool writes_done_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    CHECK(reader_writer_ != nullptr);
    writes_done_succeeded = reader_writer_->WritesDone();
  }
  if (!writes_done_succeeded) {
    // Normally the capture thread waits until service stops sending messages,
    // but in this case since we failed to notify the service we pull emergency
    // stop plug. Setting this flag forces capture thread to exit as soon
    // as it notices that it was set.
    ERROR(
        "WritesDone on Capture's gRPC stream failed: unable to finish the "
        "capture in orderly manner, initiating emergency stop");
    writes_done_failed_ = true;
  } else {
    LOG("Finished writing on Capture's gRPC stream: asking to stop capturing");
  }

  return true;
}

bool CaptureClient::AbortCaptureAndWait(int64_t max_wait_ms) {
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (client_context_ == nullptr) {
      LOG("AbortCaptureAndWait ignored: no ClientContext to TryCancel");
      return false;
    }
    LOG("Calling TryCancel on Capture's gRPC context: aborting the capture");
    try_abort_ = true;
    client_context_->TryCancel();  // reader_writer_->Read in Capture should then fail
  }

  // With this wait we want to leave at least some time for FinishCapture to be called, so that
  // reader_writer_ and in particular client_context_ are destroyed before returning to the caller.
  {
    absl::MutexLock lock(&state_mutex_);
    state_mutex_.AwaitWithTimeout(
        absl::Condition(
            +[](State* state) { return *state == State::kStopped; }, &state_),
        absl::Milliseconds(max_wait_ms));
  }
  return true;
}

ErrorMessageOr<void> CaptureClient::FinishCapture() {
  ORBIT_SCOPE_FUNCTION;

  grpc::Status status;
  {
    absl::WriterMutexLock lock{&context_and_stream_mutex_};
    CHECK(reader_writer_ != nullptr);
    status = reader_writer_->Finish();
    reader_writer_.reset();
    CHECK(client_context_ != nullptr);
    client_context_.reset();
  }

  {
    absl::MutexLock lock(&state_mutex_);
    state_ = State::kStopped;
  }

  if (!status.ok()) {
    ERROR("Finishing gRPC call to Capture: %s", status.error_message());
    return ErrorMessage{status.error_message()};
  }
  return outcome::success();
}
