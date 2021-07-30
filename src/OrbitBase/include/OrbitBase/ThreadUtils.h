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

static constexpr uint32_t kInvalidThreadId = 0xffffffff;
static constexpr uint32_t kInvalidProcessId = 0xffffffff;

// Platform-agnostic thread and process ids are represented by a uint32_t. Platform-specific invalid
// ids are converted to kInvalidThreadId or kInvalidProcessId which are the same on all platform.
[[nodiscard]] uint32_t GetCurrentThreadId();
[[nodiscard]] uint32_t GetCurrentProcessId();
void SetCurrentThreadName(const char* thread_name);
[[nodiscard]] std::string GetThreadName(uint32_t tid);

// Platform specific.
#ifdef _WIN32
[[nodiscard]] uint32_t GetCurrentThreadIdNative();
[[nodiscard]] std::string GetThreadNameNative(uint32_t tid);
[[nodiscard]] uint32_t GetCurrentProcessIdNative();

[[nodiscard]] uint32_t GetThreadIdFromNative(uint32_t tid);
[[nodiscard]] uint32_t GetProcessIdFromNative(uint32_t pid);

#else
[[nodiscard]] pid_t GetCurrentThreadIdNative();
[[nodiscard]] std::string GetThreadNameNative(pid_t tid);
[[nodiscard]] pid_t GetCurrentProcessIdNative();

[[nodiscard]] uint32_t GetThreadIdFromNative(pid_t tid);
[[nodiscard]] uint32_t GetProcessIdFromNative(pid_t pid);

[[nodiscard]] pid_t GetNativeThreadId(uint32_t tid);
[[nodiscard]] pid_t GetNativeProcessId(uint32_t tid);
#endif

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_UTILS_H_
