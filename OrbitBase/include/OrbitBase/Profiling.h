// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#include "absl/time/time.h"

#ifdef _WIN32
#include <Windows.h>
using pid_t = uint32_t;
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
#ifdef _WIN32
  __int64 time;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  return static_cast<uint64_t>(time) * 100;
#else
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1000000000ll * ts.tv_sec + ts.tv_nsec;
#endif
}

[[nodiscard]] inline absl::Duration TicksToDuration(uint64_t start, uint64_t end) {
  return absl::Nanoseconds(end - start);
}

[[nodiscard]] inline double TicksToMicroseconds(uint64_t start, uint64_t end) {
  return static_cast<double>((end - start)) * 0.001;
}

#ifdef __linux__
[[nodiscard]] inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif

#endif  // ORBIT_BASE_PROFILING_H_
