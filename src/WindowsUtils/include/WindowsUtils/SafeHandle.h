// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_SAFE_HANDLE_H_
#define WINDOWS_UTILS_SAFE_HANDLE_H_

#include "OrbitBase/Result.h"
#include "OrbitBase/UniqueResource.h"

// clang-format off
#include <Windows.h>
#include <handleapi.h>
// clang-format on

namespace orbit_windows_utils {

using SafeHandleBase = orbit_base::unique_resource<HANDLE, void (*)(HANDLE)>;

// Wrapper around a Windows "HANDLE" which calls "CloseHandle" on destruction if non-null.
struct SafeHandle : public SafeHandleBase {
  SafeHandle() = default;
  explicit SafeHandle(HANDLE handle);
  [[nodiscard]] HANDLE operator*() const { return get(); }
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_SAFE_HANDLE_H_