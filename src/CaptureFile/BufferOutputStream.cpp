// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BufferOutputStream.h"

#include <algorithm>
#include <limits>

#include "OrbitBase/File.h"

namespace orbit_capture_file_internal {

bool BufferOutputStream::Next(void** data, int* size) {
  CHECK(data != nullptr);
  CHECK(size != nullptr);
  absl::MutexLock lock{&mutex_};

  size_t old_size = buffer_.size();

  size_t new_size;
  if (old_size < buffer_.capacity()) {
    // Resize the vector to match its capacity, since we can get away
    // without a memory allocation this way.
    new_size = buffer_.capacity();
  } else {
    // Size has reached capacity, try to double it.
    new_size = old_size * 2;
  }
  // Avoid integer overflow in returned '*size'.
  new_size = std::min(new_size, old_size + std::numeric_limits<int>::max());
  // Make sure the increase size is at least kMinimumSize. Here "+ 0" works around GCC4 weirdness.
  new_size = std::max(new_size, kMinimumSize + 0);
  // Grow the vector.
  buffer_.resize(new_size);

  *data = buffer_.data() + old_size;
  *size = buffer_.size() - old_size;

  return true;
}

void BufferOutputStream::BackUp(int count) {
  CHECK(count >= 0);
  absl::MutexLock lock{&mutex_};
  CHECK(static_cast<size_t>(count) <= buffer_.size());

  buffer_.resize(buffer_.size() - count);
}

google::protobuf::int64 BufferOutputStream::ByteCount() const {
  absl::MutexLock lock{&mutex_};

  return buffer_.size();
}

void BufferOutputStream::Swap(std::vector<unsigned char>& target) {
  absl::MutexLock lock{&mutex_};

  buffer_.swap(target);
}

const std::vector<unsigned char>& BufferOutputStream::Read() const {
  absl::MutexLock lock{&mutex_};

  return buffer_;
}

}  // namespace orbit_capture_file_internal