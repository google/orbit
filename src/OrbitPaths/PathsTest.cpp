// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitPaths/Paths.h"

namespace orbit_paths {

TEST(Path, AllAutoCreatedDirsExist) {
  auto test_fns = {CreateOrGetOrbitAppDataDir, CreateOrGetDumpDir,    CreateOrGetPresetDir,
                   CreateOrGetCacheDir,        CreateOrGetCaptureDir, CreateOrGetLogDir};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Paths, AllDirsOfFilesExist) {
  auto test_fns = {GetLogFilePath};

  for (auto fn : test_fns) {
    std::filesystem::path path = fn().parent_path();
    LOG("Testing existence of \"%s\"", path.string());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

}  // namespace orbit_paths
