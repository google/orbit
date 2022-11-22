// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "ExtractSignalFromMinidump.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_linux_capture_service {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(ExtractSignalFromMinidump, ExtractSignal4) {
  std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_ggp_stand.10849.1657182631.core.dmp";
  const auto signal_or_error = ExtractSignalFromMinidump(file_path);
  // The core file in testdata was produced by a `kill -4 pid`.
  EXPECT_THAT(signal_or_error, HasValue(4));
}

TEST(ExtractSignalFromMinidump, BrokenCoreFile) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "broken.core.dmp";
  const auto signal_or_error = ExtractSignalFromMinidump(file_path);
  EXPECT_THAT(signal_or_error, HasError("Unexpected end of data."));
}

TEST(ExtractSignalFromMinidump, EmptyCoreFile) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "empty.core.dmp";
  const auto signal_or_error = ExtractSignalFromMinidump(file_path);
  EXPECT_THAT(signal_or_error, HasError("No termination signal found in core file."));
}

TEST(ExtractSignalFromMinidump, CoreFileDoesntExist) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "doesnt_exist.core.dmp";
  const auto signal_or_error = ExtractSignalFromMinidump(file_path);
  EXPECT_THAT(signal_or_error, HasError("No such file or directory"));
}

}  // namespace orbit_linux_capture_service
