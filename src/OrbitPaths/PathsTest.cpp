// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <initializer_list>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitPaths/Paths.h"
#include "TestUtils/TestUtils.h"

namespace orbit_paths {

TEST(Path, AllAutoCreatedDirsExistUnsafe) {
  auto test_fns = {CreateOrGetOrbitAppDataDirUnsafe, CreateOrGetDumpDirUnsafe,
                   CreateOrGetPresetDirUnsafe,       CreateOrGetCacheDirUnsafe,
                   CreateOrGetCaptureDirUnsafe,      CreateOrGetLogDirUnsafe,
                   CreateOrGetOrbitUserDataDirUnsafe};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn();
    ORBIT_LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllAutoCreatedDirsExist) {
  auto test_fns = {CreateOrGetOrbitUserDataDir,
                   CreateOrGetCaptureDir,
                   CreateOrGetPresetDir,
                   CreateOrGetOrbitAppDataDir,
                   CreateOrGetCacheDir,
                   CreateOrGetDumpDir,
                   CreateOrGetLogDir};

  for (auto fn : test_fns) {
    ErrorMessageOr<std::filesystem::path> path_or_error = fn();
    ASSERT_THAT(path_or_error, orbit_test_utils::HasValue());
    EXPECT_TRUE(std::filesystem::is_directory(path_or_error.value()));
  }
}

TEST(Paths, AllDirsOfFilesExistUnsafe) {
  auto test_fns = {GetLogFilePathUnsafe};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn().parent_path();
    ORBIT_LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Paths, AllDirsOfFilesExist) {
  auto test_fns = {GetLogFilePath, GetSymbolsFilePath};

  for (auto fn : test_fns) {
    ErrorMessageOr<std::filesystem::path> path_or_error = fn();
    ASSERT_THAT(path_or_error, orbit_test_utils::HasValue());
    EXPECT_TRUE(std::filesystem::is_directory(path_or_error.value().parent_path()));
  }
}

}  // namespace orbit_paths
