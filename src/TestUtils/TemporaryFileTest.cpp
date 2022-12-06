// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"

namespace orbit_test_utils {

using testing::HasSubstr;
using testing::Not;

TEST(TemporaryFile, Smoke) {
  auto tmp_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(tmp_file_or_error.has_value()) << tmp_file_or_error.error().message();
  TemporaryFile tmp_file = std::move(tmp_file_or_error.value());

  EXPECT_TRUE(tmp_file.fd().valid());
  EXPECT_THAT(tmp_file.file_path().string(), HasSubstr("orbit_"));
  EXPECT_THAT(tmp_file.file_path().string(), Not(HasSubstr("XXXXXX")));

  std::filesystem::path file_path_copy = tmp_file.file_path();

  std::error_code error;
  EXPECT_TRUE(std::filesystem::exists(tmp_file.file_path(), error));
  EXPECT_FALSE(error) << error.message();

  tmp_file.CloseAndRemove();
  EXPECT_FALSE(std::filesystem::exists(tmp_file.file_path(), error));
  EXPECT_FALSE(error) << error.message();
  EXPECT_FALSE(tmp_file.fd().valid());
  EXPECT_EQ(tmp_file.file_path(), file_path_copy);
}

TEST(TemporaryFile, MoveCtor) {
  auto tmp_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(tmp_file_or_error.has_value());
  TemporaryFile tmp_file = std::move(tmp_file_or_error.value());
  std::filesystem::path file_path_copy = tmp_file.file_path();
  int fd_value_copy = tmp_file.fd().get();

  std::error_code error;
  EXPECT_TRUE(std::filesystem::exists(tmp_file.file_path(), error));
  EXPECT_FALSE(error) << error.message();
  {
    TemporaryFile tmp_file_2 = std::move(tmp_file);
    EXPECT_TRUE(std::filesystem::exists(tmp_file_2.file_path(), error));
    EXPECT_FALSE(error) << error.message();

    EXPECT_EQ(tmp_file_2.file_path(), file_path_copy);
    EXPECT_EQ(tmp_file_2.fd().get(), fd_value_copy);

    EXPECT_EQ(tmp_file.file_path(), "");  // NOLINT: intentionally checking moved object.
    EXPECT_FALSE(tmp_file.fd().valid());  // NOLINT: intentionally checking moved object.
  }
  EXPECT_FALSE(std::filesystem::exists(file_path_copy, error));
  EXPECT_FALSE(error) << error.message();
}

TEST(TemporaryFile, MoveAssign) {
  auto tmp_file_or_error1 = TemporaryFile::Create();
  auto tmp_file_or_error2 = TemporaryFile::Create();
  ASSERT_TRUE(tmp_file_or_error1.has_value()) << tmp_file_or_error1.error().message();
  ASSERT_TRUE(tmp_file_or_error2.has_value()) << tmp_file_or_error2.error().message();
  TemporaryFile tmp_file1 = std::move(tmp_file_or_error1.value());
  TemporaryFile tmp_file2 = std::move(tmp_file_or_error2.value());

  std::filesystem::path file_path_copy1 = tmp_file1.file_path();

  std::filesystem::path file_path_copy2 = tmp_file2.file_path();
  int fd_value_copy2 = tmp_file2.fd().get();

  std::error_code error;
  EXPECT_TRUE(std::filesystem::exists(tmp_file1.file_path(), error));
  EXPECT_FALSE(error) << error.message();

  EXPECT_TRUE(std::filesystem::exists(tmp_file2.file_path(), error));
  EXPECT_FALSE(error) << error.message();

  tmp_file1 = std::move(tmp_file2);

  EXPECT_FALSE(std::filesystem::exists(file_path_copy1, error));
  EXPECT_FALSE(error) << error.message();

  EXPECT_TRUE(std::filesystem::exists(file_path_copy2, error));
  EXPECT_FALSE(error) << error.message();

  EXPECT_EQ(tmp_file2.file_path(), "");  // NOLINT: intentionally checking moved object.
  EXPECT_FALSE(tmp_file2.fd().valid());  // NOLINT: intentionally checking moved object.

  EXPECT_EQ(tmp_file1.file_path(), file_path_copy2);
  ASSERT_TRUE(tmp_file1.fd().valid());
  EXPECT_EQ(tmp_file1.fd().get(), fd_value_copy2);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif  // __GNUC__
TEST(TemporaryFile, SelfMove) {
  auto tmp_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(tmp_file_or_error.has_value());
  TemporaryFile tmp_file = std::move(tmp_file_or_error.value());
  std::filesystem::path file_path_copy = tmp_file.file_path();
  int fd_value_copy = tmp_file.fd().get();

  tmp_file = std::move(tmp_file);

  EXPECT_EQ(tmp_file.file_path(), file_path_copy);
  EXPECT_TRUE(tmp_file.fd().valid());
  EXPECT_EQ(tmp_file.fd().get(), fd_value_copy);

  std::error_code error;
  EXPECT_TRUE(std::filesystem::exists(file_path_copy, error));
  EXPECT_FALSE(error) << error.message();
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

TEST(TemporaryFile, Cleanup) {
  std::filesystem::path file_path_copy;
  {
    auto tmp_file_or_error = TemporaryFile::Create();
    ASSERT_TRUE(tmp_file_or_error.has_value()) << tmp_file_or_error.error().message();
    TemporaryFile tmp_file = std::move(tmp_file_or_error.value());

    EXPECT_TRUE(tmp_file.fd().valid());
    EXPECT_THAT(tmp_file.file_path().string(), HasSubstr("orbit_"));
    EXPECT_THAT(tmp_file.file_path().string(), Not(HasSubstr("XXXXXX")));
    file_path_copy = tmp_file.file_path();
  }

  std::error_code error;
  EXPECT_FALSE(std::filesystem::exists(file_path_copy, error));
  EXPECT_FALSE(error) << error.message();
}

TEST(TemporaryFile, CleanupAfterReopen) {
  std::filesystem::path file_path_copy;
  {
    auto tmp_file_or_error = TemporaryFile::Create();
    ASSERT_TRUE(tmp_file_or_error.has_value()) << tmp_file_or_error.error().message();
    TemporaryFile tmp_file = std::move(tmp_file_or_error.value());

    EXPECT_TRUE(tmp_file.fd().valid());
    EXPECT_THAT(tmp_file.file_path().string(), HasSubstr("orbit_"));
    EXPECT_THAT(tmp_file.file_path().string(), Not(HasSubstr("XXXXXX")));
    file_path_copy = tmp_file.file_path();

    // Check that the file is removed even if we call RemoveAndClose and then recreate it.
    tmp_file.CloseAndRemove();
    auto fd_or_error = orbit_base::OpenFileForWriting(file_path_copy);
    ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  }

  std::error_code error;
  EXPECT_FALSE(std::filesystem::exists(file_path_copy, error));
  EXPECT_FALSE(error) << error.message();
}

TEST(TemporaryFile, CustomPrefix) {
  const std::string_view custom_prefix = "custom prefix";
  auto tmp_file_or_error = TemporaryFile::Create(custom_prefix);
  ASSERT_TRUE(tmp_file_or_error.has_value()) << tmp_file_or_error.error().message();
  TemporaryFile tmp_file = std::move(tmp_file_or_error.value());

  EXPECT_TRUE(tmp_file.fd().valid());
  EXPECT_THAT(tmp_file.file_path().string(), HasSubstr(custom_prefix));
  EXPECT_THAT(tmp_file.file_path().string(), Not(HasSubstr("XXXXXX")));

  std::filesystem::path file_path_copy = tmp_file.file_path();

  std::error_code error;
  EXPECT_TRUE(std::filesystem::exists(tmp_file.file_path(), error));
  EXPECT_FALSE(error) << error.message();

  tmp_file.CloseAndRemove();
  EXPECT_FALSE(std::filesystem::exists(tmp_file.file_path(), error));
  EXPECT_FALSE(error) << error.message();
  EXPECT_FALSE(tmp_file.fd().valid());
  EXPECT_EQ(tmp_file.file_path(), file_path_copy);
}

}  // namespace orbit_test_utils