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
  auto result = orbit_windows_utils::OpenProcessForReading(orbit_base::GetCurrentProcessId());
  ASSERT_TRUE(result.has_value());
  SafeHandle safe_handle = std::move(result.value());
}

TEST(SafeHandle, NullHandle) { SafeHandle safe_handle(nullptr); }

TEST(SafeHandle, DoubleCloseError) {
  HANDLE handle = nullptr;
  {
    auto result = orbit_windows_utils::OpenProcessForReading(orbit_base::GetCurrentProcessId());
    ASSERT_TRUE(result.has_value());
    SafeHandle& safe_handle = result.value();
    handle = safe_handle.get();
  }

  // CloseHandle returns 0 on error.
  EXPECT_EQ(CloseHandle(handle), 0);
}
