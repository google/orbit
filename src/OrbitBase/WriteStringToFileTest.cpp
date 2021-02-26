// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <outcome.hpp>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/WriteStringToFile.h"

namespace orbit_base {
TEST(WriteStringToFile, Smoke) {
  // TODO(http://b/180574275): Replace this with temporary_file once it becomes available
  const char* temp_file_name = std::tmpnam(nullptr);
  const char* full_content = "content\nnew line(this text is not written)";
  constexpr size_t kContentSize = 16;
  const char* expected_content = "content\nnew line";
  std::string_view content(full_content, kContentSize);

  ErrorMessageOr<void> result = WriteStringToFile(temp_file_name, content);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  ErrorMessageOr<std::string> actual_content_or_error = ReadFileToString(temp_file_name);
  ASSERT_FALSE(actual_content_or_error.has_error()) << actual_content_or_error.error().message();
  EXPECT_EQ(actual_content_or_error.value(), expected_content);
  remove(temp_file_name);  // TODO(http://b/180574275): see above
}
}  // namespace orbit_base