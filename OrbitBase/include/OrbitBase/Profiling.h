// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#include <string>

[[nodiscard]] inline uint64_t MonotonicTimestampNs();
[[nodiscard]] inline uint32_t GetCurrentThreadId();
[[nodiscard]] inline std::string GetThreadName(uint32_t tid);
inline void SetCurrentThreadName(const std::string& thread_name);

#ifdef _WIN32
#include "ProfilingWindows.inc"
#else
#include "ProfilingLinux.inc"
#endif

#endif  // ORBIT_BASE_PROFILING_H_
