// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_READ_PROCESS_MEMORY_H_
#define WINDOWS_UTILS_READ_PROCESS_MEMORY_H_

#include <string>

#include "OrbitBase/Result.h"

namespace orbit_windows_utils {

[[nodiscard]] ErrorMessageOr<std::string> ReadProcessMemory(uint32_t pid, const void* address,
                                                            uint64_t size);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_READ_PROCESS_MEMORY_H_
