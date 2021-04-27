// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFileOutputStream.h"

#include <absl/base/casts.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <optional>
#include <string>

#include "CaptureFileConstants.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_capture_file {

namespace {

class CaptureFileOutputStreamImpl final : public CaptureFileOutputStream {
 public:
  explicit CaptureFileOutputStreamImpl(std::filesystem::path path) : path_{std::move(path)} {}
  ~CaptureFileOutputStreamImpl() noexcept override;

  [[nodiscard]] ErrorMessageOr<void> Initialize();
  [[nodiscard]] ErrorMessageOr<void> WriteCaptureEvent(
      const orbit_grpc_protos::ClientCaptureEvent& event) override;
  [[nodiscard]] ErrorMessageOr<void> Close() noexcept override;
  [[nodiscard]] bool IsOpen() noexcept override;

 private:
  void Reset() noexcept;
  [[nodiscard]] ErrorMessageOr<void> WriteHeader();
  // Handles write error by cleaning up the file and generating error message.
  [[nodiscard]] ErrorMessage HandleWriteError(const char* section_name,
                                              std::string_view original_error);
  // Call this in case of unrecoverable error to close and remove the file.
  void CloseAndTryRemoveFileAfterError();

  std::filesystem::path path_;
  orbit_base::unique_fd fd_;

  std::optional<google::protobuf::io::FileOutputStream> file_output_stream_;
  std::optional<google::protobuf::io::CodedOutputStream> coded_output_;
};

CaptureFileOutputStreamImpl::~CaptureFileOutputStreamImpl() noexcept {
  // The destructor is not default to make sure close for streams and the file are called in
  // the correct order.
  Reset();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::Initialize() {
  auto fd_or_error = orbit_base::OpenFileForWriting(path_);
  if (fd_or_error.has_error()) {
    return fd_or_error.error();
  }

  fd_ = std::move(fd_or_error.value());

  if (auto result = WriteHeader(); result.has_error()) {
    return result.error();
  }

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::Close() noexcept {
  coded_output_->Trim();
  if (coded_output_->HadError()) {
    return HandleWriteError("Unknown", SafeStrerror(file_output_stream_->GetErrno()));
  }
  Reset();

  return outcome::success();
}

void CaptureFileOutputStreamImpl::Reset() noexcept {
  coded_output_.reset();
  file_output_stream_.reset();
  fd_.release();
}

bool CaptureFileOutputStreamImpl::IsOpen() noexcept { return fd_.valid(); }

void CaptureFileOutputStreamImpl::CloseAndTryRemoveFileAfterError() {
  Reset();
  if (remove(path_.string().c_str()) == -1) {
    ERROR("Unable to remove \"%s\": %s", path_.string(), SafeStrerror(errno));
  }
}

ErrorMessage CaptureFileOutputStreamImpl::HandleWriteError(const char* section_name,
                                                           std::string_view original_error) {
  CloseAndTryRemoveFileAfterError();
  return ErrorMessage{absl::StrFormat(R"(Error writing "%s" section to "%s": %s)", section_name,
                                      path_.string(), original_error)};
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::WriteCaptureEvent(
    const orbit_grpc_protos::ClientCaptureEvent& event) {
  CHECK(coded_output_.has_value());
  CHECK(file_output_stream_.has_value());
  size_t message_size = event.ByteSizeLong();
  coded_output_->WriteVarint64(message_size);
  if (!event.SerializeToCodedStream(&coded_output_.value())) {
    return HandleWriteError("Capture", SafeStrerror(file_output_stream_->GetErrno()));
  }

  if (coded_output_->HadError()) {
    return HandleWriteError("Capture", SafeStrerror(file_output_stream_->GetErrno()));
  }

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::WriteHeader() {
  CHECK(fd_.valid());

  std::string header{kFileSignature, kFileSignatureSize};
  header.append(std::string_view(absl::bit_cast<char*>(&kFileVersion), sizeof(kFileVersion)));
  // signature - 4bytes, version - 4bytes
  // capture section offset - 8 bytes
  // additional section offset - 8 bytes
  uint64_t capture_section_offset =
      kFileSignatureSize + sizeof(kFileVersion) + 2 * sizeof(uint64_t);
  header.append(std::string_view(absl::bit_cast<char*>(&capture_section_offset),
                                 sizeof(capture_section_offset)));
  uint64_t additional_section_list_offset =
      0;  // For the streaming format we have no additional sections
  header.append(std::string_view(absl::bit_cast<char*>(&additional_section_list_offset),
                                 sizeof(additional_section_list_offset)));

  CHECK(capture_section_offset == header.size());

  auto write_result = orbit_base::WriteFully(fd_, header);
  if (write_result.has_error()) {
    return HandleWriteError("Header", write_result.error().message());
  }

  // Prepare the protobuf stream to use to write to capture section.
  file_output_stream_.emplace(fd_.get());
  coded_output_.emplace(&file_output_stream_.value());

  return outcome::success();
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureFileOutputStream>> CaptureFileOutputStream::Create(
    std::filesystem::path path) {
  auto implementation = std::make_unique<CaptureFileOutputStreamImpl>(std::move(path));
  auto init_result = implementation->Initialize();
  if (init_result.has_error()) {
    return init_result.error();
  }

  return implementation;
}

}  // namespace orbit_capture_file