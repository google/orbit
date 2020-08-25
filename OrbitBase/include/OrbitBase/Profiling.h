// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#include <absl/base/internal/sysinfo.h>

#include "absl/time/time.h"

#ifdef _WIN32
#include <Windows.h>
using pid_t = uint32_t;
#define CLOCK_MONOTONIC 1
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef _WIN32
inline void clock_gettime(uint32_t, struct timespec* spec) {
  __int64 time;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  spec->tv_sec = time / 10000000i64;
  spec->tv_nsec = time % 10000000i64 * 100;
}
#endif

inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1000000000ll * ts.tv_sec + ts.tv_nsec;
}

inline uint64_t TicksToNanoseconds(uint64_t start, uint64_t end) { return end - start; }

inline absl::Duration TicksToDuration(uint64_t start, uint64_t end) {
  return absl::Nanoseconds(TicksToNanoseconds(start, end));
}

inline double TicksToMicroseconds(uint64_t a_Start, uint64_t a_End) {
  return double((a_End - a_Start)) * 0.001;
}

inline uint64_t MicrosecondsToTicks(double a_Micros) {
  return static_cast<uint64_t>(a_Micros * 1000);
}

#ifdef __linux__
inline pid_t GetThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif

#endif  // ORBIT_BASE_PROFILING_H_
