// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#ifdef _WIN32
#include <Windows.h>
using pid_t = uint32_t;
#else
#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#endif

#ifdef _WIN32
[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  __int64 time;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  return static_cast<uint64_t>(time) * 100;
}
#else
[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1000000000LL * ts.tv_sec + ts.tv_nsec;
}

[[nodiscard]] inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif

#endif  // ORBIT_BASE_PROFILING_H_
