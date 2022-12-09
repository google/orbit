// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#include <profileapi.h>
#else
#include <time.h>
#endif

namespace orbit_base {

// CaptureTimestampNs() provides timestamps to place events on Orbit's main capture timeline.
// Event producers on a given platform should all be part of the same time domain for their events
// to be correlatable to other profiling events in Orbit. CaptureTimestatmpNs() produces timestamps
// that increase monotonically.

#ifdef _WIN32

// Retrieves the period of the performance counter from a call to "QueryPerformanceFrequency()".
// From Microsoft's documentation: "The frequency of the performance counter is fixed at system boot
// and is consistent across all processors. Therefore, the frequency need only be queried upon
// application initialization, and the result can be cached." A typical value is 100ns.
[[nodiscard]] inline uint64_t GetPerformanceCounterPeriodNs() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return 1'000'000'000 / frequency.QuadPart;
}

// Converts a "raw" timestamp retrieved from "QueryPerformanceCounter()" to nanoseconds.
[[nodiscard]] inline uint64_t PerformanceCounterToNs(uint64_t raw_timestamp) {
  static uint64_t performance_counter_period_ns_ = GetPerformanceCounterPeriodNs();
  return raw_timestamp * performance_counter_period_ns_;
}

[[nodiscard]] inline uint64_t CaptureTimestampNs() {
  LARGE_INTEGER ticks;
  QueryPerformanceCounter(&ticks);
  return PerformanceCounterToNs(ticks.QuadPart);
}

#else

// Linux clock associated with Orbit's main time domain.
constexpr clockid_t kOrbitCaptureClock = CLOCK_MONOTONIC;

[[nodiscard]] inline uint64_t CaptureTimestampNs() {
  timespec ts;
  clock_gettime(kOrbitCaptureClock, &ts);
  return 1'000'000'000LL * ts.tv_sec + ts.tv_nsec;
}

#endif

// Estimates the clock resolution for debugging purposes. Should only be used to display this
// information to the user or log it.
[[nodiscard]] uint64_t EstimateClockResolution();

// Print result of "EstimateClockResolution()" to log and return resolution value.
[[nodiscard]] uint64_t EstimateAndLogClockResolution();

}  // namespace orbit_base

#endif  // ORBIT_BASE_PROFILING_H_
