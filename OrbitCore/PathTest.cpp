// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>

#include "OrbitBase/Logging.h"
#include "Path.h"
#include "absl/strings/match.h"

TEST(Path, FileExistsEmptyFilename) {
  std::string filename{};
  EXPECT_FALSE(std::filesystem::exists(filename));
}

TEST(Path, FileExistsRootDir) {
  std::string filename;
#ifdef _WIN32
  filename = "C:\\";
#else
  filename = "/";
#endif
  EXPECT_TRUE(std::filesystem::exists(filename));
}

#ifndef _WIN32
TEST(Path, FileExistsDevNull) {
  std::string filename = "/dev/null";
  EXPECT_TRUE(std::filesystem::exists(filename));
}
#endif

TEST(Path, AllAutoCreatedDirsExist) {
  auto test_fns = {Path::CreateOrGetOrbitAppDataDir, Path::CreateOrGetDumpDir,
                   Path::CreateOrGetPresetDir, Path::CreateOrGetCacheDir,
                   Path::CreateOrGetOrbitAppDataDir};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllDirsOfFilesExist) {
  auto test_fns = {Path::GetLogFilePathAndCreateDir};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn().parent_path();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}
