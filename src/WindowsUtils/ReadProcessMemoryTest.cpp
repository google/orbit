// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>

#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ReadProcessMemory.h"

namespace orbit_windows_utils {

TEST(ReadProcessMemory, ReadCurrentProcessMemory) {
  std::string test_string("The quick brown fox jumps over the lazy dog");
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uintptr_t address = absl::bit_cast<uintptr_t>(test_string.data());

  std::string buffer(test_string.size(), {});
  uint64_t num_bytes_read = 0;
  uint64_t size = test_string.size();
  ErrorMessageOr<void> result =
      orbit_windows_utils::ReadProcessMemory(pid, address, buffer.data(), size, &num_bytes_read);

  EXPECT_FALSE(result.has_error());
  EXPECT_STREQ(buffer.data(), test_string.data());
}

TEST(ReadProcessMemory, ReadCurrentProcessMemoryAsString) {
  std::string test_string("The quick brown fox jumps over the lazy dog");
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<std::string> result = orbit_windows_utils::ReadProcessMemory(
      pid, absl::bit_cast<uintptr_t>(test_string.data()), test_string.size());
  EXPECT_FALSE(result.has_error());
  EXPECT_STREQ(result.value().data(), test_string.data());
}

TEST(ReadProcessMemory, ReadInvalidProcessMemory) {
  uint32_t pid = orbit_base::kInvalidProcessId;
  constexpr const size_t kReadSize = 32;
  ErrorMessageOr<std::string> result =
      orbit_windows_utils::ReadProcessMemory(pid, /*address=*/0, kReadSize);
  EXPECT_TRUE(result.has_error());
}

}  // namespace orbit_windows_utils
