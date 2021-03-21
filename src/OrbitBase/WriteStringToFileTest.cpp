// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <outcome.hpp>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/TemporaryFile.h"
#include "OrbitBase/WriteStringToFile.h"

namespace orbit_base {
TEST(WriteStringToFile, Smoke) {
  auto temporary_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string temp_file_name = temporary_file.file_path().string();

  const char* full_content = "content\nnew line(this text is not written)";
  constexpr size_t kContentSize = 16;
  const char* expected_content = "content\nnew line";
  std::string_view content(full_content, kContentSize);

  ErrorMessageOr<void> result = WriteStringToFile(temp_file_name, content);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  ErrorMessageOr<std::string> actual_content_or_error = ReadFileToString(temp_file_name);
  ASSERT_FALSE(actual_content_or_error.has_error()) << actual_content_or_error.error().message();
  EXPECT_EQ(actual_content_or_error.value(), expected_content);
}
}  // namespace orbit_base