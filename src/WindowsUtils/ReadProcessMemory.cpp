// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ReadProcessMemory.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <windows.h>

#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/SafeHandle.h"

// clang-format off
#include <memoryapi.h>
// clang-format on

#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

ErrorMessageOr<void> ReadProcessMemory(uint32_t pid, uintptr_t address, void* buffer, uint64_t size,
                                       uint64_t* num_bytes_read) {
  OUTCOME_TRY(SafeHandle process_handle, OpenProcessForReading(pid));
  BOOL result = ::ReadProcessMemory(*process_handle, absl::bit_cast<void*>(address), buffer, size,
                                    num_bytes_read);

  if (result == FALSE) {
    return ErrorMessage(
        absl::StrFormat("Could not read %u bytes at address %p of process %u", size, address, pid));
  }

  return outcome::success();
}

ErrorMessageOr<std::string> ReadProcessMemory(uint32_t pid, uintptr_t address, uint64_t size) {
  std::string buffer(size, {});
  uint64_t num_bytes_read = 0;
  OUTCOME_TRY(ReadProcessMemory(pid, address, buffer.data(), size, &num_bytes_read));
  buffer.resize(num_bytes_read);
  return std::move(buffer);
}

}  // namespace orbit_windows_utils
