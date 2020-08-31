// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureClient.h"

#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "absl/flags/flag.h"

ABSL_DECLARE_FLAG(uint16_t, sampling_rate);
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);

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
    ThreadPool* thread_pool, int32_t process_id, std::string process_name,
    std::shared_ptr<Process> process,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
    absl::flat_hash_set<TracepointInfo, HashTracepointInfo, EqualTracepointInfo>
        selected_tracepoints) {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStopped) {
    return ErrorMessage(
        "Capture cannot be started, the previous capture is still "
        "running/stopping.");
  }

  state_ = State::kStarting;
  thread_pool->Schedule(
      [this, process_id, process_name, process, selected_functions, selected_tracepoints]() {
        Capture(process_id, process_name, process, selected_functions, selected_tracepoints);
      });

  return outcome::success();
}

void CaptureClient::Capture(
    int32_t process_id, std::string process_name, std::shared_ptr<Process> process,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
    absl::flat_hash_set<orbit_grpc_protos::TracepointInfo, HashTracepointInfo, EqualTracepointInfo>
        selected_tracepoints) {
  CHECK(reader_writer_ == nullptr);

  grpc::ClientContext context;
  reader_writer_ = capture_service_->Capture(&context);

  CaptureRequest request;
  CaptureOptions* capture_options = request.mutable_capture_options();
  capture_options->set_trace_context_switches(true);
  capture_options->set_pid(process_id);
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

  capture_options->set_trace_gpu_driver(true);
  for (const auto& pair : selected_functions) {
    const FunctionInfo& function = pair.second;
    CaptureOptions::InstrumentedFunction* instrumented_function =
        capture_options->add_instrumented_functions();
    instrumented_function->set_file_path(function.loaded_module_path());
    instrumented_function->set_file_offset(FunctionUtils::Offset(function));
    instrumented_function->set_absolute_address(FunctionUtils::GetAbsoluteAddress(function));
    instrumented_function->set_function_type(IntrumentedFunctionTypeFromOrbitType(function.type()));
  }

  for (const auto& tracepoint : selected_tracepoints) {
    CaptureOptions::InstrumentedTracepoint* instrumented_tracepoint =
        capture_options->add_instrumented_tracepoint();
    instrumented_tracepoint->set_category(tracepoint.category());
    instrumented_tracepoint->set_name(tracepoint.name());
  }

  if (!reader_writer_->Write(request)) {
    ERROR("Sending CaptureRequest on Capture's gRPC stream");
    reader_writer_->WritesDone();
    FinishCapture();
    return;
  }
  LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start "
      "capturing");

  state_mutex_.Lock();
  state_ = State::kStarted;
  state_mutex_.Unlock();

  CaptureEventProcessor event_processor(capture_listener_);

  capture_listener_->OnCaptureStarted(process_id, std::move(process_name), std::move(process),
                                      std::move(selected_functions),
                                      std::move(selected_tracepoints));

  CaptureResponse response;
  while (!force_stop_ && reader_writer_->Read(&response)) {
    event_processor.ProcessEvents(response.capture_events());
  }
  LOG("Finished reading from Capture's gRPC stream: all capture data has been "
      "received");

  capture_listener_->OnCaptureComplete();
  FinishCapture();
}

bool CaptureClient::StopCapture() {
  absl::MutexLock lock(&state_mutex_);
  if (state_ == State::kStarting) {
    // Block and wait until the state is not kStarting
    bool (*IsNotStarting)(State * state) = [](State* state) { return *state != State::kStarting; };
    state_mutex_.Await(absl::Condition(IsNotStarting, &state_));
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
    force_stop_ = true;
  } else {
    LOG("Finished writing on Capture's gRPC stream: asking to stop capturing");
  }

  state_ = State::kStopping;
  return true;
}

void CaptureClient::FinishCapture() {
  if (reader_writer_ == nullptr) {
    return;
  }

  grpc::Status status = reader_writer_->Finish();
  if (!status.ok()) {
    ERROR("Finishing gRPC call to Capture: %s", status.error_message());
  }
  reader_writer_.reset();

  absl::MutexLock lock(&state_mutex_);
  state_ = State::kStopped;
}
