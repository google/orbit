// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProtoSectionInputStreamImpl.h"

#include <absl/strings/str_format.h>

#include <memory>

#include "OrbitBase/MakeUniqueForOverwrite.h"

namespace orbit_capture_file_internal {

constexpr uint64_t kMaximumMessageSize = 1024 * 1024;  // 1Mb

ErrorMessageOr<void> ProtoSectionInputStreamImpl::ReadMessage(google::protobuf::Message* message) {
  // CodedInputStream imposes a hard limit on the total number of bytes it will read. It's INT_MAX
  // by default and it cannot be increased past that. To work around the limitation, reinitialize
  // the CodedInputStream, as the actual current position is kept by the FileFragmentInputStream
  // instead. Note that this makes CodedInputStream::CurrentPosition not always reflect the actual
  // position in the stream.
  if (coded_input_stream_->CurrentPosition() >= kCodedInputStreamReinitializationThreshold) {
    coded_input_stream_.emplace(&file_fragment_input_stream_);
    coded_input_stream_->SetTotalBytesLimit(kCodedInputStreamTotalBytesLimit);
  }

  uint32_t message_size = 0;

  // Note that in case there was an error CodedInputStream does not provide error messages/codes.
  // We need to go to underlying stream (file_fragment_input_stream_ in this case) to get the error
  // message in case of a failure.
  if (!coded_input_stream_->ReadVarint32(&message_size)) {
    return file_fragment_input_stream_.GetLastError().value_or(
        ErrorMessage{"Unexpected end of section while reading message size"});
  }

  // Since file input is not trusted, having too big value here may lead to out-of-memory allocation
  // below. Do a sanity check for message size, we limit our messages to 1Mb maximum size.
  if (message_size > kMaximumMessageSize) {
    return ErrorMessage{
        absl::StrFormat("The message size %d is too big (maximum allowed message size is %d)",
                        message_size, kMaximumMessageSize)};
  }

  auto buf = make_unique_for_overwrite<uint8_t[]>(message_size);
  if (!coded_input_stream_->ReadRaw(buf.get(), message_size)) {
    return file_fragment_input_stream_.GetLastError().value_or(
        ErrorMessage{"Unexpected end of section while reading the message"});
  }

  message->ParseFromArray(buf.get(), message_size);

  if (message->ByteSizeLong() != message_size) {
    return ErrorMessage{absl::StrFormat(
        "The message size %d of the parsed message is different from the parsed size %d",
        message->ByteSizeLong(), message_size)};
  }

  return outcome::success();
}

}  // namespace orbit_capture_file_internal