// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

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

inline absl::Duration TicksToDuration(uint64_t start, uint64_t end) {
  return absl::Nanoseconds(end - start);
}

inline double TicksToMicroseconds(uint64_t start, uint64_t end) {
  return double((end - start)) * 0.001;
}

#ifdef __linux__
inline pid_t GetThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif

#endif  // ORBIT_BASE_PROFILING_H_
