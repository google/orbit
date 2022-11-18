// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_UTILS_H_
#define ORBIT_GL_GL_UTILS_H_

#include <absl/time/time.h>
#include <stdint.h>

[[nodiscard]] inline absl::Duration TicksToDuration(uint64_t start, uint64_t end) {
  return absl::Nanoseconds(end - start);
}

[[nodiscard]] inline double TicksToMicroseconds(uint64_t start, uint64_t end) {
  return static_cast<double>(end - start) * 0.001;
}

#endif  // ORBIT_GL_GL_UTILS_H_
