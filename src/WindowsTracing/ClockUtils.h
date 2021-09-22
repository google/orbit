// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_CLOCK_UTILS_H_
#define WINDOWS_TRACING_CLOCK_UTILS_H_

#include <profileapi.h>

#include <cstdint>

namespace orbit_windows_tracing::clock_utils {

[[nodiscard]] inline uint64_t GetPerformanceCounterPeriodNs() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return 1'000'000'000 / frequency.QuadPart;
}

[[nodiscard]] inline uint64_t RawTimestampToNs(uint64_t raw_timestamp) {
  static uint64_t performance_period_ns_ = GetPerformanceCounterPeriodNs();
  return raw_timestamp * performance_period_ns_;
}

}  // namespace orbit_windows_tracing::clock_utils

#endif  // WINDOWS_TRACING_CLOCK_UTILS_H_
