// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/FindDebugSymbols.h"

namespace orbit_windows_utils {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(ProcessServiceUtils, ExistingPdbFile) {
  const std::filesystem::path test_directory = orbit_test::GetTestdataDir();

  const std::filesystem::path module_path = test_directory / "dllmain.dll";
  const std::filesystem::path symbols_path =
      test_directory / "additional_directory" / "dllmain.pdb";
  const ErrorMessageOr<std::filesystem::path> result =
      FindDebugSymbols(module_path, {test_directory / "additional_directory"});
  ASSERT_THAT(result, HasValue());
  EXPECT_EQ(result.value(), symbols_path);
}

TEST(ProcessServiceUtils, CorruptedPdb) {
  const std::filesystem::path test_directory = orbit_test::GetTestdataDir();

  const std::filesystem::path module_path = test_directory / "other.dll";
  // "other.pdb" is a text file that acts as a fake corrupted pdb.
  const ErrorMessageOr<std::filesystem::path> result =
      FindDebugSymbols(module_path, {test_directory / "additional_directory"});
  EXPECT_THAT(result, HasError("does not contain symbols"));
}

TEST(ProcessServiceUtils, FileDoesNotExist) {
  const std::filesystem::path test_directory = orbit_test::GetTestdataDir();

  const std::filesystem::path module_path = test_directory / "not_existing_file";
  const ErrorMessageOr<std::filesystem::path> result =
      FindDebugSymbols(module_path, {test_directory});
  EXPECT_THAT(result, HasError("Unable to load"));
}

}  // namespace orbit_windows_utils
