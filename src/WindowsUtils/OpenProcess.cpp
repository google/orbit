// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/OpenProcess.h"

#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"

// clang-format off
#include <processthreadsapi.h>
// clang-format on

namespace orbit_windows_utils {

ErrorMessageOr<SafeHandle> OpenProcess(uint32_t desired_access, bool inherit_handle,
                                       uint32_t process_id) {
  HANDLE process_handle = ::OpenProcess(desired_access, inherit_handle ? TRUE : FALSE, process_id);
  if (process_handle == nullptr) {
    return ErrorMessage(absl::StrFormat("Could not get handle for process %u: %s", process_id,
                                        orbit_base::GetLastErrorAsString()));
  }
  return SafeHandle(process_handle);
}

ErrorMessageOr<SafeHandle> OpenProcessForReading(uint32_t process_id) {
  return OpenProcess(PROCESS_VM_READ, /*inherit_handle=*/false, process_id);
}

}  // namespace orbit_windows_utils