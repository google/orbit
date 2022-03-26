// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>

#include "OrbitBase/ThreadUtils.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/SafeHandle.h"
#include "WindowsUtils/WriteProcessMemory.h"

using orbit_windows_utils::WriteProcessMemory;

TEST(WriteProcessMemory, WriteCurrentProcessWithProcessId) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  const std::string test_string("The quick brown fox jumps over the lazy dog");
  std::string destination_buffer(2048, 0);
  auto result =
      orbit_windows_utils::WriteProcessMemory(process_id, destination_buffer.data(), test_string);

  EXPECT_THAT(result, orbit_test_utils::HasNoError());
  EXPECT_STREQ(destination_buffer.data(), test_string.c_str());
}

TEST(WriteProcessMemory, WriteCurrentProcessWithProcessHandle) {
  orbit_windows_utils::SafeHandle process_handle(GetCurrentProcess());
  const std::string test_string("The quick brown fox jumps over the lazy dog");
  std::string destination_buffer(2048, 0);
  auto result = orbit_windows_utils::WriteProcessMemory(*process_handle, destination_buffer.data(),
                                                        test_string);

  EXPECT_THAT(result, orbit_test_utils::HasNoError());
  EXPECT_STREQ(destination_buffer.data(), test_string.c_str());
}

TEST(WriteProcessMemory, WriteToInvalidMemoryLocation) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  const std::string test_string("The quick brown fox jumps over the lazy dog");
  auto result = orbit_windows_utils::WriteProcessMemory(pid, nullptr, test_string);
  EXPECT_THAT(result, orbit_test_utils::HasError("Invalid access to memory location"));
}
