// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_THREAD_UTILS_H_
#define ORBIT_BASE_THREAD_UTILS_H_

#include <cstdint>
#include <string>

namespace orbit_base {

[[nodiscard]] uint32_t GetCurrentThreadId();
[[nodiscard]] std::string GetThreadName(uint32_t tid);
void SetCurrentThreadName(const std::string& thread_name);

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_UTILS_H_
