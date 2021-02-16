// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_THREAD_UTILS_H_
#define ORBIT_BASE_THREAD_UTILS_H_

#ifdef _WIN32
#include <stdint.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string>

namespace orbit_base {

#ifdef _WIN32
[[nodiscard]] uint32_t GetCurrentThreadId();
[[nodiscard]] std::string GetThreadName(uint32_t tid);
[[nodiscard]] uint32_t GetCurrentProcessId();
#else
[[nodiscard]] pid_t GetCurrentThreadId();
[[nodiscard]] std::string GetThreadName(pid_t tid);
[[nodiscard]] pid_t GetCurrentProcessId();
#endif

void SetCurrentThreadName(const char* thread_name);

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_UTILS_H_
