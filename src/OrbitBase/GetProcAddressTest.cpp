// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "OrbitBase/GetProcAddress.h"

namespace orbit_base {

TEST(GetProcAddress, FindExistingFunctions) {
  static auto set_thread_description =
      GetProcAddress<HRESULT(WINAPI*)(HANDLE, PCWSTR)>("kernel32.dll", "SetThreadDescription");
  EXPECT_NE(set_thread_description, nullptr);

  static auto fatal_exit = GetProcAddress<void(WINAPI*)(int)>("kernel32.dll", "FatalExit");
  EXPECT_NE(fatal_exit, nullptr);

  static auto is_zoomed = GetProcAddress<BOOL(WINAPI*)(HWND)>("user32.dll", "IsZoomed");
  EXPECT_NE(is_zoomed, nullptr);
}

TEST(GetProcAddress, NonExistingModule) {
  static auto invalid_function =
      GetProcAddress<void (*)()>("non_existing.dll", "non_existing_function_name");
  EXPECT_EQ(invalid_function, nullptr);
}

TEST(GetProcAddress, NonExistingFunction) {
  static auto invalid_function =
      GetProcAddress<void (*)()>("kernel32.dll", "non_existing_function_name");
  EXPECT_EQ(invalid_function, nullptr);
}

}  // namespace orbit_base
