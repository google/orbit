// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_BUSY_LOOP_UTILS_H_
#define WINDOWS_UTILS_BUSY_LOOP_UTILS_H_

// clang-format off
#include <Windows.h>
// clang-format on

#include <stdint.h>
#include <string>
#include "OrbitBase/Result.h"

namespace orbit_windows_utils {

// Information returned from "InstallBusyLoopAtAddress" required to resume the original code.
struct BusyLoopInfo {
  uint32_t process_id = 0;
  uint64_t address = 0;
  std::string original_bytes;
};

ErrorMessageOr<BusyLoopInfo> InstallBusyLoopAtAddress(HANDLE process_handle, void* address);
ErrorMessageOr<void> RemoveBusyLoop(const BusyLoopInfo& busy_loop_info);

ErrorMessageOr<void> SuspendThread(HANDLE thread_handle);
ErrorMessageOr<void> ResumeThread(HANDLE thread_handle);
ErrorMessageOr<void> SetThreadInstructionPointer(HANDLE thread_handle,
                                                 uint64_t instruction_pointer);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_BUSY_LOOP_UTILS_H_
