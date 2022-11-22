// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FileFragmentInputStream.h"

#include <stdint.h>

#include <utility>

namespace orbit_capture_file_internal {

using orbit_base::ReadFullyAtOffset;

bool FileFragmentInputStream::Next(const void** data, int* size) {
  ORBIT_CHECK(data != nullptr);
  ORBIT_CHECK(size != nullptr);

  if (last_error_.has_value()) return false;

  uint64_t bytes_to_read = std::min(buffer_.size(), file_fragments_end_ - current_position_);

  auto bytes_read_or_error =
      ReadFullyAtOffset(fd_, buffer_.data(), bytes_to_read, current_position_);
  if (bytes_read_or_error.has_error()) {
    last_error_ = std::move(bytes_read_or_error.error());
    return false;
  }

  uint64_t bytes_read = bytes_read_or_error.value();
  // Might happen in the case when file is truncated or
  // file_fragments_end_ is beyond EOF for some reason.
  if (bytes_read == 0) {
    return false;
  }

  current_position_ += bytes_read;
  (*data) = buffer_.data();
  (*size) = static_cast<int>(bytes_read);

  return true;
}

void FileFragmentInputStream::BackUp(int count) {
  ORBIT_CHECK(count >= 0);
  if (static_cast<size_t>(count) > current_position_) {
    current_position_ = file_fragments_start_;
    return;
  }

  current_position_ = std::max(file_fragments_start_, current_position_ - count);
}

bool FileFragmentInputStream::Skip(int count) {
  ORBIT_CHECK(count >= 0);

  if (last_error_.has_value()) return false;

  current_position_ = std::min(file_fragments_end_, current_position_ + count);
  return current_position_ < file_fragments_end_;
}

int64_t FileFragmentInputStream::ByteCount() const {
  return current_position_ - file_fragments_start_;
}

}  // namespace orbit_capture_file_internal