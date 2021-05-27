// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <initializer_list>
#include <string>

#include "OrbitBase/Logging.h"
#include "Path.h"

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
  auto test_fns = {orbit_core::CreateOrGetOrbitAppDataDir, orbit_core::CreateOrGetDumpDir,
                   orbit_core::CreateOrGetPresetDir,       orbit_core::CreateOrGetCacheDir,
                   orbit_core::CreateOrGetCaptureDir,      orbit_core::CreateOrGetLogDir};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllDirsOfFilesExist) {
  auto test_fns = {orbit_core::GetLogFilePath};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn().parent_path();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}
