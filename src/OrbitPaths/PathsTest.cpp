// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitPaths/Paths.h"
#include "TestUtils/TestUtils.h"

namespace orbit_paths {

TEST(Path, AllAutoCreatedDirsExist) {
  auto test_fns = {CreateOrGetOrbitAppDataDir, CreateOrGetDumpDir,    CreateOrGetPresetDir,
                   CreateOrGetCacheDir,        CreateOrGetCaptureDir, CreateOrGetLogDir,
                   CreateOrGetOrbitUserDataDir};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn();
    ORBIT_LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllAutoCreatedDirsExistSafe) {
  auto test_fns = {CreateOrGetOrbitUserDataDirSafe,
                   CreateOrGetCaptureDirSafe,
                   CreateOrGetPresetDirSafe,
                   CreateOrGetOrbitAppDataDirSafe,
                   CreateOrGetCacheDirSafe,
                   CreateOrGetDumpDirSafe,
                   CreateOrGetLogDirSafe};

  for (auto fn : test_fns) {
    ErrorMessageOr<std::filesystem::path> path_or_error = fn();
    ASSERT_THAT(path_or_error, orbit_test_utils::HasValue());
    EXPECT_TRUE(std::filesystem::is_directory(path_or_error.value()));
  }
}

TEST(Paths, AllDirsOfFilesExist) {
  auto test_fns = {GetLogFilePath};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn().parent_path();
    ORBIT_LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Paths, AllDirsOfFilesExistSafe) {
  auto test_fns = {GetLogFilePathSafe};

  for (auto fn : test_fns) {
    ErrorMessageOr<std::filesystem::path> path_or_error = fn();
    ASSERT_THAT(path_or_error, orbit_test_utils::HasValue());
    EXPECT_TRUE(std::filesystem::is_directory(path_or_error.value().parent_path()));
  }
}

}  // namespace orbit_paths
