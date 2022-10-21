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

// We use uint32_t as our cross-platform thread id and process id type. Platform-specific invalid
// ids are converted into cross-platform kInvalidThreadId or kInvalidProcessId (both 0xffffffff):
//
// +----------------------------------------------------------------------+
// |                             Thread Id                                |
// +-------------+-----------------+---------------------+----------------+
// |             | Windows         | Linux               | Cross Platform |
// +-------------+-----------------+---------------------+----------------+
// | Type        | uint32_t        | int32_t             | uint32_t       |
// +-------------+-----------------+---------------------+----------------+
// | Valid Range | [4, 2^32-4]     | [0, 2^31-1]         | [0, 2^32-4]    |
// +-------------+-----------------+---------------------+----------------+
// | Invalid Id  | 0               | -1                  | 0xffffffff     |
// +-------------+-----------------+---------------------+----------------+
// | Note        | Multiples of 4  | “Swapper” has tid 0 |                |
// +-------------+-----------------+---------------------+----------------+
//
//
// +----------------------------------------------------------------------+
// |                            Process Id                                |
// +-------------+-----------------+---------------------+----------------+
// |             | Windows         | Linux               | Cross Platform |
// +-------------+-----------------+---------------------+----------------+
// | Type        | uint32_t        | int32_t             | uint32_t       |
// +-------------+-----------------+---------------------+----------------+
// | Valid Range | [4, 2^32-4]     | [0, 2^31-1]         | [0, 2^32-4]    |
// +-------------+-----------------+---------------------+----------------+
// | Invalid Id  | 0, 0xffffffff   | -1                  | 0xffffffff     |
// +-------------+-----------------+---------------------+----------------+
// | Note        | Multiples of 4  | “Swapper” has pid 0 |                |
// +-------------+-----------------+---------------------+----------------+

// Cross-platform.
[[nodiscard]] uint32_t GetCurrentThreadId();
[[nodiscard]] uint32_t GetCurrentProcessId();

[[nodiscard]] bool IsValidThreadId(uint32_t tid);
[[nodiscard]] bool IsValidProcessId(uint32_t pid);

void SetCurrentThreadName(const char* thread_name);
[[nodiscard]] std::string GetThreadName(uint32_t tid);

// Platform-specific.
#ifdef _WIN32
[[nodiscard]] uint32_t GetCurrentThreadIdNative();
[[nodiscard]] std::string GetThreadNameNative(uint32_t tid);
[[nodiscard]] uint32_t GetCurrentProcessIdNative();

[[nodiscard]] uint32_t FromNativeThreadId(uint32_t tid);
[[nodiscard]] uint32_t FromNativeProcessId(uint32_t pid);

#else
[[nodiscard]] pid_t GetCurrentThreadIdNative();
[[nodiscard]] std::string GetThreadNameNative(pid_t tid);
[[nodiscard]] pid_t GetCurrentProcessIdNative();

[[nodiscard]] uint32_t FromNativeThreadId(pid_t tid);
[[nodiscard]] uint32_t FromNativeProcessId(pid_t pid);

[[nodiscard]] pid_t ToNativeThreadId(uint32_t tid);
[[nodiscard]] pid_t ToNativeProcessId(uint32_t tid);
#endif

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_UTILS_H_
