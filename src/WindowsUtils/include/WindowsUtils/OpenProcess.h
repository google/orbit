// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_OPEN_PROCESS_H_
#define WINDOWS_UTILS_OPEN_PROCESS_H_

#include "WindowsUtils/SafeHandle.h"

namespace orbit_windows_utils {

// Wrapper around Windows' "OpenProcess" function which either returns a "SafeHandle" or an error.
ErrorMessageOr<SafeHandle> OpenProcess(uint32_t desired_access, bool inherit_handle,
                                       uint32_t process_id);
// Calls OpenProcess(PROCESS_VM_READ, /*inherit_handle=*/false, process_id).
ErrorMessageOr<SafeHandle> OpenProcessForReading(uint32_t process_id);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_OPEN_PROCESS_H_