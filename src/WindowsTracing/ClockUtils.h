// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_CLOCK_UTILS_H_
#define WINDOWS_TRACING_CLOCK_UTILS_H_

#include <profileapi.h>

#include <cstdint>

namespace orbit_windows_tracing {

class ClockUtils {
 public:
  [[nodiscard]] static inline uint64_t RawTimestampToNs(uint64_t raw_timestamp) {
    return raw_timestamp * GetInstance().performance_period_ns_;
  }

 private:
  ClockUtils() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    performance_frequency_ = frequency.QuadPart;
    performance_period_ns_ = 1'000'000'000 / performance_frequency_;
  }

  [[nodiscard]] static ClockUtils& GetInstance() {
    static ClockUtils clock_utils;
    return clock_utils;
  }

  uint64_t performance_frequency_;
  uint64_t performance_period_ns_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_CLOCK_UTILS_H_
