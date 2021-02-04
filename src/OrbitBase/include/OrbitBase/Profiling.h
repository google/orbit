// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#ifdef _WIN32
#include <chrono>
#else
#include <stdint.h>
#include <time.h>
#endif

#ifdef _WIN32
[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  auto time_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(time_since_epoch).count();
}
#else
[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1000000000LL * ts.tv_sec + ts.tv_nsec;
}
#endif

#endif  // ORBIT_BASE_PROFILING_H_
