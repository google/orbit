// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/SafeHandle.h"

namespace orbit_windows_utils {

namespace {

// Wrapper around "CloseHandle" which supports nullptr.
void SafeCloseHandle(HANDLE handle) {
  if (handle) {
    ::CloseHandle(handle);
  }
}

}  // namespace

SafeHandle::SafeHandle(HANDLE handle) : SafeHandleBase(handle, SafeCloseHandle) {}

}  // namespace orbit_windows_utils