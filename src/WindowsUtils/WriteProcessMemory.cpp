// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/WriteProcessMemory.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"
#include "WindowsUtils/OpenProcess.h"

// clang-format off
#include <Windows.h>
#include <memoryapi.h>
// clang-format on

#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

// Write "buffer" at memory location "address" of process identified by "process_handle".
ErrorMessageOr<void> WriteProcessMemory(HANDLE process_handle, void* address,
                                        absl::Span<const char> buffer) {
  SIZE_T num_bytes_written = 0;
  BOOL result = ::WriteProcessMemory(process_handle, address, buffer.data(), buffer.size(),
                                     &num_bytes_written);

  if (result == 0 || num_bytes_written != buffer.size()) {
    return ErrorMessage(absl::StrFormat(
        "Could not write %u bytes at address %p of process %u. %u byte(s) were written: %s",
        buffer.size(), address, GetProcessId(process_handle), num_bytes_written,
        orbit_base::GetLastErrorAsString()));
  }

  return outcome::success();
}

ErrorMessageOr<void> WriteProcessMemory(uint32_t pid, void* address,
                                        absl::Span<const char> buffer) {
  OUTCOME_TRY(SafeHandle process_handle,
              OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, pid));

  return WriteProcessMemory(*process_handle, address, buffer);
}

}  // namespace orbit_windows_utils
