// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <utility>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureFile/CaptureFileOutputStream.h"
#include "ClientProtos/user_defined_capture_info.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

using orbit_capture_file::CaptureFileOutputStream;
using orbit_client_protos::UserDefinedCaptureInfo;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_capture_client {

namespace {

class SaveToFileEventProcessor : public CaptureEventProcessor {
 public:
  explicit SaveToFileEventProcessor(std::filesystem::path file_path,
                                    std::function<void(const ErrorMessage&)> error_handler)
      : file_path_{std::move(file_path)},
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

  void ReportError(const ErrorMessage& error);

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
  ORBIT_CHECK(output_stream_ != nullptr);

  if (state_ == State::kCaptureFinished) {
    ReportError(ErrorMessage{"Unexpected event after CaptureFinished event"});
    return;
  }

  if (state_ == State::kErrorReported) return;

  ORBIT_CHECK(output_stream_->IsOpen());

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

    state_ = State::kCaptureFinished;
  }
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureEventProcessor>>
CaptureEventProcessor::CreateSaveToFileProcessor(
    const std::filesystem::path& file_path,
    std::function<void(const ErrorMessage&)> error_handler) {
  auto processor = std::make_unique<SaveToFileEventProcessor>(file_path, std::move(error_handler));
  auto init_or_error = processor->Initialize();
  if (init_or_error.has_error()) {
    return init_or_error.error();
  }

  return processor;
}

}  // namespace orbit_capture_client
