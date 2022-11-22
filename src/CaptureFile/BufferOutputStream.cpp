// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/BufferOutputStream.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_capture_file {

bool BufferOutputStream::Write(const void* data, int size) {
  ORBIT_CHECK(data != nullptr);
  ORBIT_CHECK(size >= 0);
  absl::MutexLock lock{&mutex_};

  size_t old_size = buffer_.size();
  size_t new_size = old_size + size;
  ORBIT_CHECK(new_size > old_size);

  buffer_.resize(new_size);
  std::memcpy(buffer_.data() + old_size, data, size);

  return true;
}

std::vector<unsigned char> BufferOutputStream::TakeBuffer() {
  absl::MutexLock lock{&mutex_};

  auto output_buffer = std::move(buffer_);

  return output_buffer;
}

}  // namespace orbit_capture_file