// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Test/Path.h"

#include <cstdlib>
#include <filesystem>
#include <string>

#include "OrbitBase/ExecutablePath.h"

namespace orbit_test {

// This is the current working directory that was set during the start of the application.
// Any later changes don't affect the value of `initial_working_directory`.
const std::filesystem::path initial_working_directory = std::filesystem::current_path();

std::filesystem::path GetTestdataDir() {
  const char* const override_path = std::getenv("ORBIT_OVERRIDE_TESTDATA_PATH");

  if (override_path != nullptr) {
    std::filesystem::path testdata_dir{std::string{override_path}};
    if (testdata_dir.is_relative()) testdata_dir = initial_working_directory / testdata_dir;
    return testdata_dir;
  }

  const auto test_name = orbit_base::GetExecutablePath().stem();
  return orbit_base::GetExecutableDir() / "testdata" / test_name;
}

}  // namespace orbit_test
