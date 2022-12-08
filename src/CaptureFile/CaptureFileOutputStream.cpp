// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFileOutputStream.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <errno.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <stdint.h>
#include <stdio.h>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "CaptureFile/BufferOutputStream.h"
#include "CaptureFileConstants.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_capture_file {

namespace {

class CaptureFileOutputStreamImpl final : public CaptureFileOutputStream {
 public:
  explicit CaptureFileOutputStreamImpl(std::filesystem::path path)
      : output_type_(OutputType::kFile), path_{std::move(path)} {}
  explicit CaptureFileOutputStreamImpl(BufferOutputStream* output_buffer)
      : output_type_(OutputType::kBuffer), output_buffer_(output_buffer) {}
  ~CaptureFileOutputStreamImpl() override;

  [[nodiscard]] ErrorMessageOr<void> Initialize();
  [[nodiscard]] ErrorMessageOr<void> WriteCaptureEvent(
      const orbit_grpc_protos::ClientCaptureEvent& event) override;
  [[nodiscard]] ErrorMessageOr<void> Close() override;
  [[nodiscard]] bool IsOpen() override;

 private:
  void Reset();
  [[nodiscard]] ErrorMessageOr<void> WriteHeader();
  [[nodiscard]] std::string_view GetErrorFromOutputStream() const;
  // Handles write error by cleaning up the file and generating error message.
  [[nodiscard]] ErrorMessage HandleWriteError(const char* section_name,
                                              std::string_view original_error);
  // Call this in case of unrecoverable error to close and remove the file.
  void CloseAndTryRemoveFileAfterError();

  enum class OutputType { kFile, kBuffer };
  OutputType output_type_;

  std::filesystem::path path_;
  orbit_base::unique_fd fd_;
  BufferOutputStream* output_buffer_ = nullptr;
  std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> zero_copy_output_stream_;
  std::optional<google::protobuf::io::CodedOutputStream> coded_output_;
};

CaptureFileOutputStreamImpl::~CaptureFileOutputStreamImpl() {
  // The destructor is not default to make sure close for streams and the file are called in
  // the correct order.
  Reset();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::Initialize() {
  // Prepare the protobuf stream to use to write to capture section.
  switch (output_type_) {
    case OutputType::kBuffer: {
      ORBIT_CHECK(output_buffer_ != nullptr);

      zero_copy_output_stream_ =
          std::make_unique<google::protobuf::io::CopyingOutputStreamAdaptor>(output_buffer_);
      break;
    }
    case OutputType::kFile: {
      auto fd_or_error = orbit_base::OpenNewFileForWriting(path_);
      if (fd_or_error.has_error()) return fd_or_error.error();
      fd_ = std::move(fd_or_error.value());
      ORBIT_CHECK(fd_.valid());

      zero_copy_output_stream_ =
          std::make_unique<google::protobuf::io::FileOutputStream>(fd_.get());
      break;
    }
  }

  coded_output_.emplace(zero_copy_output_stream_.get());

  if (auto result = WriteHeader(); result.has_error()) {
    return result.error();
  }

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::Close() {
  coded_output_->Trim();
  if (coded_output_->HadError()) {
    return HandleWriteError("Unknown", GetErrorFromOutputStream());
  }
  Reset();

  return outcome::success();
}

void CaptureFileOutputStreamImpl::Reset() {
  // Destroy coded_output_ before destroying the underlaying ZeroCopyOutputStream to guarantee that
  // coded_output_ flushes all data to the underlaying ZeroCopyOutputStream and trims the unused
  // bytes.
  coded_output_.reset();
  zero_copy_output_stream_.reset(nullptr);
  fd_.release();
  output_buffer_ = nullptr;
}

bool CaptureFileOutputStreamImpl::IsOpen() {
  switch (output_type_) {
    case OutputType::kBuffer:
      return output_buffer_ != nullptr;
    case OutputType::kFile:
      return fd_.valid();
  }

  ORBIT_UNREACHABLE();
}

void CaptureFileOutputStreamImpl::CloseAndTryRemoveFileAfterError() {
  Reset();

  if (output_type_ == OutputType::kFile && remove(path_.string().c_str()) == -1) {
    ORBIT_ERROR("Unable to remove \"%s\": %s", path_.string(), SafeStrerror(errno));
  }
}

std::string_view CaptureFileOutputStreamImpl::GetErrorFromOutputStream() const {
  // There should not be any write error in the case of `OutputType::kBuffer` as we do not limit the
  // buffer size of BufferOutputStream.
  ORBIT_CHECK(output_type_ == OutputType::kFile);
  auto* file_output_stream =
      static_cast<google::protobuf::io::FileOutputStream*>(zero_copy_output_stream_.get());
  return SafeStrerror(file_output_stream->GetErrno());
}

ErrorMessage CaptureFileOutputStreamImpl::HandleWriteError(const char* section_name,
                                                           std::string_view original_error) {
  CloseAndTryRemoveFileAfterError();

  std::string output_target = output_type_ == OutputType::kFile ? path_.string() : "";
  return ErrorMessage{absl::StrFormat(R"(Error writing "%s" section to "%s": %s)", section_name,
                                      output_target, original_error)};
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::WriteCaptureEvent(
    const orbit_grpc_protos::ClientCaptureEvent& event) {
  ORBIT_CHECK(coded_output_.has_value());
  ORBIT_CHECK(zero_copy_output_stream_ != nullptr);

  uint32_t event_size = event.ByteSizeLong();
  coded_output_->WriteVarint32(event_size);
  if (!event.SerializeToCodedStream(&coded_output_.value()) || coded_output_->HadError()) {
    return HandleWriteError("Capture", GetErrorFromOutputStream());
  }

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileOutputStreamImpl::WriteHeader() {
  ORBIT_CHECK(coded_output_.has_value());

  std::string header{kFileSignature};
  header.append(std::string_view(absl::bit_cast<char*>(&kFileVersion), sizeof(kFileVersion)));
  // signature - 4bytes, version - 4bytes
  // capture section offset - 8 bytes
  // additional section offset - 8 bytes
  uint64_t capture_section_offset =
      kFileSignature.size() + sizeof(kFileVersion) + 2 * sizeof(uint64_t);
  header.append(std::string_view(absl::bit_cast<char*>(&capture_section_offset),
                                 sizeof(capture_section_offset)));
  uint64_t additional_section_list_offset =
      0;  // For the streaming format we have no additional sections
  header.append(std::string_view(absl::bit_cast<char*>(&additional_section_list_offset),
                                 sizeof(additional_section_list_offset)));

  ORBIT_CHECK(capture_section_offset == header.size());

  coded_output_->WriteString(header);
  if (coded_output_->HadError()) {
    return HandleWriteError("Header", GetErrorFromOutputStream());
  }

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

std::unique_ptr<CaptureFileOutputStream> CaptureFileOutputStream::Create(
    BufferOutputStream* output_buffer) {
  auto implementation = std::make_unique<CaptureFileOutputStreamImpl>(output_buffer);
  auto init_result = implementation->Initialize();
  ORBIT_CHECK(!init_result.has_error());

  return implementation;
}

}  // namespace orbit_capture_file