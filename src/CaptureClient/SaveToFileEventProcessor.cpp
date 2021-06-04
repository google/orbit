// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureFile/CaptureFile.h"
#include "CaptureFile/CaptureFileHelpers.h"
#include "CaptureFile/CaptureFileOutputStream.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "capture_data.pb.h"

using orbit_capture_file::CaptureFileOutputStream;
using orbit_client_protos::UserDefinedCaptureInfo;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_capture_client {

namespace {

class SaveToFileEventProcessor : public CaptureEventProcessor {
 public:
  explicit SaveToFileEventProcessor(std::filesystem::path file_path,
                                    absl::flat_hash_set<uint64_t> frame_track_function_ids,
                                    std::function<void(const ErrorMessage&)> error_handler)
      : frame_track_function_ids_(std::move(frame_track_function_ids)),
        file_path_{std::move(file_path)},
        error_handler_{std::move(error_handler)},
        state_{State::kProcessing} {}
  ~SaveToFileEventProcessor() override = default;

  ErrorMessageOr<void> Initialize();
  void ProcessEvent(const ClientCaptureEvent& event) override;

 private:
  enum class State {
    kProcessing,
    kCaptureFinished,
    kErrorReported,
  };

  ErrorMessageOr<void> WriteUserData();
  void ReportError(const ErrorMessage& error);

  absl::flat_hash_set<uint64_t> frame_track_function_ids_;
  std::filesystem::path file_path_;
  std::function<void(const ErrorMessage&)> error_handler_;
  std::unique_ptr<CaptureFileOutputStream> output_stream_;
  State state_;
};

ErrorMessageOr<void> SaveToFileEventProcessor::Initialize() {
  auto stream_or_error = CaptureFileOutputStream::Create(file_path_);
  if (stream_or_error.has_error()) {
    return ErrorMessage{absl::StrFormat("Failed to initialize CaptureSaveToFileProcessor: %s",
                                        stream_or_error.error().message())};
  }

  output_stream_ = std::move(stream_or_error.value());

  return outcome::success();
}

void SaveToFileEventProcessor::ReportError(const ErrorMessage& error) {
  error_handler_(error);
  state_ = State::kErrorReported;
}

void SaveToFileEventProcessor::ProcessEvent(const ClientCaptureEvent& event) {
  CHECK(output_stream_ != nullptr);

  if (state_ == State::kCaptureFinished) {
    ReportError(ErrorMessage{"Unexpected event after CaptureFinished event"});
    return;
  }

  if (state_ == State::kErrorReported) return;

  CHECK(output_stream_->IsOpen());

  auto write_result = output_stream_->WriteCaptureEvent(event);
  if (write_result.has_error()) {
    ReportError(write_result.error());
    return;
  }

  if (event.event_case() == ClientCaptureEvent::kCaptureFinished) {
    // We are done - close the stream.
    auto close_result = output_stream_->Close();
    if (close_result.has_error()) {
      ReportError(close_result.error());
      return;
    }

    auto write_user_data_result = WriteUserData();
    if (write_user_data_result.has_error()) {
      ReportError(write_user_data_result.error());
      return;
    }

    state_ = State::kCaptureFinished;
  }
}

ErrorMessageOr<void> SaveToFileEventProcessor::WriteUserData() {
  UserDefinedCaptureInfo user_defined_capture_data;
  for (uint64_t function_id : frame_track_function_ids_) {
    user_defined_capture_data.mutable_frame_tracks_info()->add_frame_track_function_ids(
        function_id);
  }

  OUTCOME_TRY(orbit_capture_file::WriteUserData(file_path_, user_defined_capture_data));

  return outcome::success();
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureEventProcessor>>
CaptureEventProcessor::CreateSaveToFileProcessor(
    const std::filesystem::path& file_path, absl::flat_hash_set<uint64_t> frame_track_function_ids,
    std::function<void(const ErrorMessage&)> error_handler) {
  auto processor = std::make_unique<SaveToFileEventProcessor>(
      file_path, std::move(frame_track_function_ids), std::move(error_handler));
  auto init_or_error = processor->Initialize();
  if (init_or_error.has_error()) {
    return init_or_error.error();
  }

  return processor;
}

}  // namespace orbit_capture_client
