// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_WRITE_PROCESS_MEMORY_H_
#define WINDOWS_UTILS_WRITE_PROCESS_MEMORY_H_

#include <absl/types/span.h>
#include <stdint.h>

#include "OrbitBase/Result.h"

// clang-format off
#include <Windows.h>
#include <handleapi.h>
// clang-format on

namespace orbit_windows_utils {

// Write "buffer" at memory location "address" of process identified by "process_id".
[[nodiscard]] ErrorMessageOr<void> WriteProcessMemory(uint32_t process_id, void* address,
                                                      absl::Span<const char> buffer);

// Write "buffer" at memory location "address" of process identified by "process_handle".
[[nodiscard]] ErrorMessageOr<void> WriteProcessMemory(HANDLE process_handle, void* address,
                                                      absl::Span<const char> buffer);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_WRITE_PROCESS_MEMORY_H_