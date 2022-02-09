// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef WIN32

#include <gtest/gtest.h>

#include "OrbitBase/GetProcAddress.h"
#include "TestUtils/TestUtils.h"
#include "Windows.h"

namespace orbit_base {

using orbit_test_utils::HasError;

TEST(GetProcAddress, FindExistingFunctions) {
  static auto set_thread_description =
      GetProcAddress<HRESULT(WINAPI*)(HANDLE, PCWSTR)>("kernel32.dll", "SetThreadDescription");
  EXPECT_NE(set_thread_description, nullptr);

  static auto fatal_exit = GetProcAddress<void(WINAPI*)(int)>("kernel32.dll", "FatalExit");
  EXPECT_NE(fatal_exit, nullptr);
}

TEST(GetProcAddress, NonExistingModule) {
  EXPECT_THAT(GetProcAddress("non_existing.dll", "non_existing_function_name"),
              HasError("Could not find module"));
}

TEST(GetProcAddress, NonExistingFunction) {
  EXPECT_THAT(GetProcAddress("kernel32.dll", "non_existing_function_name"),
              HasError("Could not find function"));
}

}  // namespace orbit_base

#endif  // WIN32