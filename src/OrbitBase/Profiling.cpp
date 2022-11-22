// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Profiling.h"

#include <algorithm>
#include <limits>

#include "OrbitBase/Logging.h"

namespace orbit_base {
uint64_t EstimateClockResolution() {
  uint64_t minimum_resolution_found = std::numeric_limits<uint64_t>::max();

  // We limit the amount of time spent to measure timer resolution.
  uint64_t iteration_start_time_ns = orbit_base::CaptureTimestampNs();
  uint64_t duration_ns = 0;
  constexpr uint64_t kMaximumDurationNs = 1000 * 1000;  // 1 ms

  while (duration_ns < kMaximumDurationNs) {
    // Count to limit the number of iterations in the while loop below. Note that the loop
    // below will terminate within one quantum of the clock used. For the unlikely (broken?)
    // case that the clock does not increase, this additional limit is checked. However, this
    // value also needs to be sufficiently large so that the loop will actually hit an
    // increase of the clock.
    constexpr int kSafetyNetCount = 1'000'000;

    uint64_t resolution = 0;
    int safety_net_counter = 0;
    while (resolution == 0 && safety_net_counter < kSafetyNetCount) {
      uint64_t start_time_ns = orbit_base::CaptureTimestampNs();
      uint64_t end_time_ns = orbit_base::CaptureTimestampNs();
      // This will be >= 0 as we use a monotonic clock.
      resolution = end_time_ns - start_time_ns;
      safety_net_counter++;
    }
    minimum_resolution_found = std::min(minimum_resolution_found, resolution);
    duration_ns = orbit_base::CaptureTimestampNs() - iteration_start_time_ns;
  }
  return minimum_resolution_found;
}

uint64_t EstimateAndLogClockResolution() {
  // We expect the value to be in the order of 35-100 nanoseconds.
  uint64_t clock_resolution_ns = orbit_base::EstimateClockResolution();
  if (clock_resolution_ns > 0) {
    ORBIT_LOG("Clock resolution: %d (ns)", clock_resolution_ns);
  } else {
    ORBIT_ERROR("Failed to estimate clock resolution");
  }

  return clock_resolution_ns;
}

}  // namespace orbit_base