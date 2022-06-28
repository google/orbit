// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/WindowsVersion.h"
#include "TestUtils/TestUtils.h"

TEST(WindowsVersion, GetWindowsVersion) {
  ASSERT_THAT(orbit_base::GetWindowsVersion(), orbit_test_utils::HasNoError());
}

TEST(WindowsVersion, GetWindowsVersionAsString) {
  auto result = orbit_base::GetWindowsVersionAsString();
  ASSERT_TRUE(result.has_value());
}
