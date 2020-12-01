// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureClient.h"

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ProcessData.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

ABSL_DECLARE_FLAG(uint16_t, sampling_rate);
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);
ABSL_DECLARE_FLAG(bool, thread_state);

using orbit_client_protos::FunctionInfo;

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::TracepointInfo;

static CaptureOptions::InstrumentedFunction::FunctionType IntrumentedFunctionTypeFromOrbitType(
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

ErrorMessageOr<void> CaptureClient::StartCapture(
    ThreadPool* thread_pool, const ProcessData& process,
    const OrbitClientData::ModuleManager& module_manager,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
    TracepointInfoSet selected_tracepoints, UserDefinedCaptureData user_defined_capture_data,
    bool enable_introspection) {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStopped) {
    return ErrorMessage(
        "Capture cannot be started, the previous capture is still "
        "running/stopping.");
  }

  state_ = State::kStarting;

  // TODO(168797897) Here a copy of the process is created. The loaded modules of this process were
  // likely filled when the process was selected, which might be a while back. Between then and now
  // the loaded modules might have changed. Up to date information about which modules are loaded
  // should be used here. (Even better: while taking a capture this should always be up to date)
  ProcessData process_copy = process;

  thread_pool->Schedule([this, process = std::move(process_copy), &module_manager,
                         selected_functions = std::move(selected_functions), selected_tracepoints,
                         user_defined_capture_data = std::move(user_defined_capture_data),
                         enable_introspection]() mutable {
    Capture(std::move(process), module_manager, std::move(selected_functions),
            std::move(selected_tracepoints), std::move(user_defined_capture_data),
            enable_introspection);
  });

  return outcome::success();
}

void CaptureClient::Capture(ProcessData&& process,
                            const OrbitClientData::ModuleManager& module_manager,
                            absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
                            TracepointInfoSet selected_tracepoints,
                            UserDefinedCaptureData user_defined_capture_data,
                            bool enable_introspection) {
  ORBIT_SCOPE_FUNCTION;
  CHECK(client_context_ == nullptr);
  CHECK(reader_writer_ == nullptr);

  writes_done_failed_ = false;
  try_abort_ = false;
  client_context_ = std::make_unique<grpc::ClientContext>();
  reader_writer_ = capture_service_->Capture(client_context_.get());

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

  capture_options->set_trace_thread_state(absl::GetFlag(FLAGS_thread_state));
  capture_options->set_trace_gpu_driver(true);
  for (const auto& [absolute_address, function] : selected_functions) {
    CaptureOptions::InstrumentedFunction* instrumented_function =
        capture_options->add_instrumented_functions();
    instrumented_function->set_file_path(function.loaded_module_path());
    const ModuleData* module = module_manager.GetModuleByPath(function.loaded_module_path());
    CHECK(module != nullptr);
    instrumented_function->set_file_offset(FunctionUtils::Offset(function, *module));
    instrumented_function->set_absolute_address(absolute_address);
    instrumented_function->set_function_type(
        IntrumentedFunctionTypeFromOrbitType(function.orbit_type()));
  }

  for (const auto& tracepoint : selected_tracepoints) {
    TracepointInfo* instrumented_tracepoint = capture_options->add_instrumented_tracepoint();
    instrumented_tracepoint->set_category(tracepoint.category());
    instrumented_tracepoint->set_name(tracepoint.name());
  }

  capture_options->set_enable_introspection(enable_introspection);

  if (!reader_writer_->Write(request)) {
    ERROR("Sending CaptureRequest on Capture's gRPC stream");
    reader_writer_->WritesDone();
    ErrorMessageOr<void> finish_result = FinishCapture();
    std::string error_string =
        absl::StrFormat("Error sending capture request.%s",
                        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    capture_listener_->OnCaptureFailed(ErrorMessage{error_string});
    return;
  }
  LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start capturing");

  state_mutex_.Lock();
  state_ = State::kStarted;
  state_mutex_.Unlock();

  CaptureEventProcessor event_processor(capture_listener_);

  capture_listener_->OnCaptureStarted(std::move(process), std::move(selected_functions),
                                      std::move(selected_tracepoints),
                                      std::move(user_defined_capture_data));

  CaptureResponse response;
  while (!writes_done_failed_ && !try_abort_ && reader_writer_->Read(&response)) {
    event_processor.ProcessEvents(response.capture_events());
  }

  ErrorMessageOr<void> finish_result = FinishCapture();
  if (try_abort_) {
    LOG("TryCancel on Capture's gRPC context was called: Read on Capture's gRPC stream failed");
    capture_listener_->OnCaptureCancelled();
  } else if (writes_done_failed_) {
    LOG("WritesDone on Capture's gRPC stream failed: stop reading and try to finish the gRPC call");
    std::string error_string = absl::StrFormat(
        "Unable to finish the capture in orderly manner, performing emergency stop.%s",
        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    capture_listener_->OnCaptureFailed(ErrorMessage{error_string});
  } else {
    LOG("Finished reading from Capture's gRPC stream: all capture data has been received");
    if (finish_result.has_error()) {
      capture_listener_->OnCaptureFailed(finish_result.error());
    } else {
      capture_listener_->OnCaptureComplete();
    }
  }
}

bool CaptureClient::StopCapture() {
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

  CHECK(reader_writer_ != nullptr);

  if (!reader_writer_->WritesDone()) {
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

  state_ = State::kStopping;
  return true;
}

bool CaptureClient::TryAbortCapture() {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStarted && state_ != State::kStopping) {
    LOG("TryAbortCapture ignored, because the capture is not started nor stopping");
    return false;
  }

  CHECK(client_context_ != nullptr);

  LOG("Calling TryCancel on Capture's gRPC context: trying to abort the capture");
  try_abort_ = true;
  client_context_->TryCancel();  // reader_writer_->Read in Capture should then fail
  return true;
}

ErrorMessageOr<void> CaptureClient::FinishCapture() {
  ORBIT_SCOPE_FUNCTION;
  if (reader_writer_ == nullptr) {
    return outcome::success();
  }

  grpc::Status status = reader_writer_->Finish();
  reader_writer_.reset();
  client_context_.reset();

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
