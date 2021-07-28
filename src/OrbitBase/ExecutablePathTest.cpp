// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "OrbitBase/ExecutablePath.h"

TEST(ExecutablePath, GetExecutablePath) {
  /* copybara:insert(executable is named differently)
  GTEST_SKIP();
  */

  std::filesystem::path path = orbit_base::GetExecutablePath();
#ifdef _WIN32
  const std::string executable_name = "OrbitBaseTests.exe";
#else
  const std::string executable_name = "OrbitBaseTests";
#endif
  EXPECT_EQ(path.filename(), executable_name);
  EXPECT_EQ(path.parent_path().filename(), "bin");
}

TEST(ExecutablePath, GetExecutableDir) {
  /* copybara:insert(executable's directory is named differently)
  GTEST_SKIP();
  */

  EXPECT_EQ(orbit_base::GetExecutableDir().filename(), "bin");
}
