// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PLATFORM_LINUX_PROFILING_H_
#define ORBIT_BASE_PLATFORM_LINUX_PROFILING_H_

#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <string>

[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1'000'000'000llu * ts.tv_sec + ts.tv_nsec;
}

[[nodiscard]] inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

[[nodiscard]] std::string GetThreadName(pid_t tid);
void SetThreadName(const std::string& thread_name);

#endif  // ORBIT_BASE_PLATFORM_LINUX_PROFILING_H_
