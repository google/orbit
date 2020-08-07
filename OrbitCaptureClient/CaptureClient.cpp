// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/CaptureClient.h"

#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"

#include "absl/flags/flag.h"

ABSL_DECLARE_FLAG(uint16_t, sampling_rate);
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);

using orbit_client_protos::FunctionInfo;

void CaptureClient::Capture(
    int32_t pid, const std::map<uint64_t, FunctionInfo*>& selected_functions) {
  CHECK(reader_writer_ == nullptr);

  event_processor_.emplace(capture_listener_);

  grpc::ClientContext context;
  reader_writer_ = capture_service_->Capture(&context);

  CaptureRequest request;
  CaptureOptions* capture_options = request.mutable_capture_options();
  capture_options->set_trace_context_switches(true);
  capture_options->set_pid(pid);
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
    const FunctionInfo* function = pair.second;
    // TODO: this is temporary fix. We should understand why in
    // Capture::GSelectedFunctionsMap could be nullptrs and fix it accordingly
    if (function == nullptr) {
      continue;
    }
    CaptureOptions::InstrumentedFunction* instrumented_function =
        capture_options->add_instrumented_functions();
    instrumented_function->set_file_path(function->loaded_module_path());
    instrumented_function->set_file_offset(FunctionUtils::Offset(*function));
    instrumented_function->set_absolute_address(
        FunctionUtils::GetAbsoluteAddress(*function));
  }

  if (!reader_writer_->Write(request)) {
    ERROR("Sending CaptureRequest on Capture's gRPC stream");
    reader_writer_->WritesDone();
    FinishCapture();
    return;
  }
  LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start "
      "capturing");

  capture_listener_->OnCaptureStarted();

  CaptureResponse response;
  while (reader_writer_->Read(&response)) {
    event_processor_->ProcessEvents(response.capture_events());
  }
  LOG("Finished reading from Capture's gRPC stream: all capture data has been "
      "received");

  capture_listener_->OnCaptureComplete();
  FinishCapture();
}

void CaptureClient::StopCapture() {
  CHECK(reader_writer_ != nullptr);

  if (!reader_writer_->WritesDone()) {
    ERROR("Finishing writing on Capture's gRPC stream");
    FinishCapture();
  }
  LOG("Finished writing on Capture's gRPC stream: asking to stop capturing");
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
  event_processor_.reset();
}