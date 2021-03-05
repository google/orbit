// Copyright (c) 2021 The Orbit Authors. All rights reserved.
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

namespace orbit_base {

// CaptureTimestampNs() provides timestamps to place events on Orbit's main capture timeline.
// Event producers on a given platform should all be part of the same time domain for their events
// to be correlatable to other profiling events in Orbit. CaptureTimestatmpNs() produces timestamps
// that increase monotonically.

#ifdef _WIN32

[[nodiscard]] inline uint64_t CaptureTimestampNs() {
  auto time_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(time_since_epoch).count();
}

#else

// Linux clock associated with Orbit's main time domain.
constexpr clockid_t kOrbitCaptureClock = CLOCK_MONOTONIC;

[[nodiscard]] inline uint64_t CaptureTimestampNs() {
  timespec ts;
  clock_gettime(kOrbitCaptureClock, &ts);
  return 1000000000LL * ts.tv_sec + ts.tv_nsec;
}

#endif

// Estimates the clock resolution for debugging purposes. Should only be used to display this
// information to the user or log it.
[[nodiscard]] uint64_t EstimateClockResolution();

}  // namespace orbit_base

#endif  // ORBIT_BASE_PROFILING_H_
