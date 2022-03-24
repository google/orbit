// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>

#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/OpenProcess.h"

TEST(OpenProcess, ValidHandleForCurrentPid) {
  auto result = orbit_windows_utils::OpenProcess(orbit_base::GetCurrentProcessId());
  EXPECT_TRUE(result.has_value());
}

TEST(OpenProcess, ErrorForInvalidPid) {
  auto result = orbit_windows_utils::OpenProcess(0);
  EXPECT_TRUE(result.has_error());
}
