// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PLATFORM_WINDOWS_PROFILING_H_
#define ORBIT_BASE_PLATFORM_WINDOWS_PROFILING_H_

#include <Windows.h>

#include <string>

using pid_t = uint32_t;

[[nodiscard]] inline uint64_t MonotonicTimestampNs() {
  __int64 time;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  return static_cast<uint64_t>(time) * 100;
}

[[nodiscard]] std::string GetThreadName(uint32_t tid);

void SetThreadName(const std::string& thread_name);

#endif  // ORBIT_BASE_PLATFORM_WINDOWS_PROFILING_H_
