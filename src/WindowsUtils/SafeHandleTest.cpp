// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>

#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/SafeHandle.h"

using orbit_windows_utils::SafeHandle;

TEST(SafeHandle, OwnershipTransfer) {
  auto result = orbit_windows_utils::OpenProcess(orbit_base::GetCurrentProcessId());
  EXPECT_TRUE(result.has_value());
  SafeHandle safe_handle = std::move(result.value());
}

TEST(SafeHandle, NullHandle) { SafeHandle safe_handle(nullptr); }

TEST(SafeHandle, DoubleCloseError) {
  HANDLE handle = nullptr;
  {
    uint32_t pid = orbit_base::GetCurrentProcessId();
    SafeHandle safe_handle =
        orbit_windows_utils::OpenProcess(PROCESS_VM_READ, /*inherit_handle=*/false, pid).value();
    handle = safe_handle.get();
  }

  // CloseHandle returns 0 on error.
  EXPECT_EQ(CloseHandle(handle), 0);
}
