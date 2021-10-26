// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BufferOutputStream.h"

#include <algorithm>
#include <limits>

#include "OrbitBase/File.h"

namespace orbit_capture_file_internal {

BufferOutputStream::BufferOutputStream(std::vector<unsigned char>* target) : target_(target) {}

bool BufferOutputStream::Next(void** data, int* size) {
  CHECK(data != nullptr);
  CHECK(size != nullptr);
  CHECK(target_ != nullptr);

  size_t old_size = target_->size();

  size_t new_size;
  if (old_size < target_->capacity()) {
    // Resize the vector to match its capacity, since we can get away
    // without a memory allocation this way.
    new_size = target_->capacity();
  } else {
    // Size has reached capacity, try to double it.
    new_size = old_size * 2;
  }
  // Avoid integer overflow in returned '*size'.
  new_size = std::min(new_size, old_size + std::numeric_limits<int>::max());
  // Make sure the increase size is at least kMinimumSize. Here "+ 0" works around GCC4 weirdness.
  new_size = std::max(new_size, kMinimumSize + 0);
  // Grow the vector.
  target_->resize(new_size);

  *data = target_->data() + old_size;
  *size = target_->size() - old_size;

  return true;
}

void BufferOutputStream::BackUp(int count) {
  CHECK(target_ != nullptr);
  CHECK(count >= 0);
  CHECK(static_cast<size_t>(count) <= target_->size());

  target_->resize(target_->size() - count);
}

int64_t BufferOutputStream::ByteCount() const {
  CHECK(target_ != nullptr);

  return target_->size();
}

}  // namespace orbit_capture_file_internal