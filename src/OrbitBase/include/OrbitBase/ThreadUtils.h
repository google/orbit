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

// Platform agnostic.
[[nodiscard]] uint32_t GetCurrentThreadId_not_native();
[[nodiscard]] uint32_t GetCurrentProcessId_not_native();
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

[[nodiscard]] uint32_t GetThreadIdFromNative(pid_t native_tid);
[[nodiscard]] uint32_t GetProcessIdFromNative(pid_t native_pid);

[[nodiscard]] pid_t GetNativeThreadId(uint32_t tid);
[[nodiscard]] pid_t GetNativeProcessId(uint32_t tid);
#endif

static constexpr uint32_t kInvalidThreadId = 0xffffffff;
static constexpr uint32_t kInvalidProcessId = 0xffffffff;

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_UTILS_H_
