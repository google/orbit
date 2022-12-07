// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <utility>

#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/WriteStringToFile.h"
#include "TestUtils/TemporaryDirectory.h"
#include "TestUtils/TestUtils.h"

namespace orbit_test_utils {

TEST(TemporaryDirectory, CreatesAndDeletesDirectory) {
  auto maybe_directory = TemporaryDirectory::Create();
  ASSERT_THAT(maybe_directory, HasNoError());

  std::filesystem::path directory_path = maybe_directory.value().GetDirectoryPath();

  EXPECT_THAT(orbit_base::IsDirectory(directory_path), HasValue(true));
  maybe_directory = ErrorMessage{};  // This ends the scope of the TemporaryDirectory instance
  EXPECT_THAT(orbit_base::FileOrDirectoryExists(directory_path), HasValue(false));
}

TEST(TemporaryDirectory, IsInitiallyEmpty) {
  auto maybe_directory = TemporaryDirectory::Create();
  ASSERT_THAT(maybe_directory, HasNoError());

  std::filesystem::path directory_path = maybe_directory.value().GetDirectoryPath();
  EXPECT_THAT(orbit_base::ListFilesInDirectory(directory_path), HasValue(testing::IsEmpty()));
}

TEST(TemporaryDirectory, CanCreateFileInTemporaryDirectoryAndDeletesIt) {
  auto maybe_directory = TemporaryDirectory::Create();
  ASSERT_THAT(maybe_directory, HasNoError());

  std::filesystem::path directory_path = maybe_directory.value().GetDirectoryPath();
  std::filesystem::path arbitrary_file_path = directory_path / "hello.txt";

  EXPECT_THAT(orbit_base::WriteStringToFile(arbitrary_file_path, "Some contents."), HasNoError());

  maybe_directory = ErrorMessage{};  // This ends the scope of the TemporaryDirectory instance
  EXPECT_THAT(orbit_base::FileOrDirectoryExists(arbitrary_file_path), HasValue(false));
}

TEST(TemporaryDirectory, Move) {
  auto maybe_directory = TemporaryDirectory::Create();
  ASSERT_THAT(maybe_directory, HasNoError());

  std::filesystem::path directory_path = maybe_directory.value().GetDirectoryPath();
  EXPECT_THAT(orbit_base::IsDirectory(directory_path), HasValue(true));

  {
    TemporaryDirectory other = std::move(maybe_directory.value());
    EXPECT_THAT(orbit_base::IsDirectory(directory_path), HasValue(true));

    // We end the lifetime of the moved-from object here. The directory should still be there.
    maybe_directory = ErrorMessage{};
    EXPECT_THAT(orbit_base::IsDirectory(directory_path), HasValue(true));
  }

  EXPECT_THAT(orbit_base::FileOrDirectoryExists(directory_path), HasValue(false));
}

}  // namespace orbit_test_utils