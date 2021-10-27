// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BufferOutputStream.h"

#include <algorithm>
#include <cstring>

#include "OrbitBase/File.h"

namespace orbit_capture_file {

bool BufferOutputStream::Write(const void* data, int size) {
  CHECK(data != nullptr);
  absl::MutexLock lock{&mutex_};

  size_t old_size = buffer_.size();
  buffer_.resize(old_size + size);
  std::memcpy(buffer_.data() + old_size, data, size);

  return true;
}

size_t BufferOutputStream::ReadIntoBuffer(void* dest, size_t max_size) {
  CHECK(dest != nullptr);
  CHECK(max_size >= 0);
  absl::MutexLock lock{&mutex_};

  size_t bytes_available_to_read = buffer_.size();
  size_t bytes_to_read = std::min(bytes_available_to_read, max_size);
  std::memcpy(dest, buffer_.data(), bytes_to_read);

  buffer_.erase(buffer_.begin(), buffer_.begin() + bytes_to_read);

  return bytes_to_read;
}

google::protobuf::int64 BufferOutputStream::BytesAvailableToRead() const {
  absl::MutexLock lock{&mutex_};

  return buffer_.size();
}

}  // namespace orbit_capture_file