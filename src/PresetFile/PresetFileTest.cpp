// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "ClientProtos/preset.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "PresetFile/PresetFile.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_preset_file {

using orbit_client_protos::PresetInfo;
using orbit_client_protos::PresetInfoLegacy;
using orbit_client_protos::PresetModule;
using orbit_client_protos::PresetModuleLegacy;
using orbit_test_utils::TemporaryFile;

static TemporaryFile CreateTemporaryFileOrDie() {
  auto temporary_file_or_error = TemporaryFile::Create();
  if (temporary_file_or_error.has_error()) {
    ORBIT_FATAL("Unable to create temporary file: %s", temporary_file_or_error.error().message());
  }

  return std::move(temporary_file_or_error.value());
}

TEST(PresetFile, SaveAndLoad) {
  auto temporary_file = CreateTemporaryFileOrDie();
  PresetInfo info;
  PresetModule module1;
  module1.add_function_names("f1_1");
  module1.add_function_names("f1_2");
  module1.add_frame_track_function_names("ft1_1");
  module1.add_frame_track_function_names("ft1_2");
  module1.add_frame_track_function_names("ft1_3");
  (*info.mutable_modules())["/path/to/module1"] = module1;

  PresetModule module2;
  module2.add_function_names("f2_1");
  module2.add_function_names("f2_2");
  module2.add_frame_track_function_names("ft2_1");
  module2.add_frame_track_function_names("ft2_2");
  module2.add_frame_track_function_names("ft2_3");
  (*info.mutable_modules())["/path/to/module2"] = module2;

  {
    PresetFile original_file{temporary_file.file_path(), info};
    EXPECT_FALSE(original_file.IsLegacyFileFormat());
    auto module_paths = original_file.GetModulePaths();
    ASSERT_EQ(module_paths.size(), 2);

    {
      auto function_names = original_file.GetSelectedFunctionNamesForModule("/path/to/module1");
      ASSERT_EQ(function_names.size(), 2);
      EXPECT_EQ(function_names.at(0), "f1_1");
      EXPECT_EQ(function_names.at(1), "f1_2");
    }

    {
      auto function_names = original_file.GetFrameTrackFunctionNamesForModule("/path/to/module1");
      ASSERT_EQ(function_names.size(), 3);
      EXPECT_EQ(function_names.at(0), "ft1_1");
      EXPECT_EQ(function_names.at(1), "ft1_2");
      EXPECT_EQ(function_names.at(2), "ft1_3");
    }

    {
      auto function_names = original_file.GetSelectedFunctionNamesForModule("/path/to/module2");
      ASSERT_EQ(function_names.size(), 2);
      EXPECT_EQ(function_names.at(0), "f2_1");
      EXPECT_EQ(function_names.at(1), "f2_2");
    }

    {
      auto function_names = original_file.GetFrameTrackFunctionNamesForModule("/path/to/module2");
      ASSERT_EQ(function_names.size(), 3);
      EXPECT_EQ(function_names.at(0), "ft2_1");
      EXPECT_EQ(function_names.at(1), "ft2_2");
      EXPECT_EQ(function_names.at(2), "ft2_3");
    }

    EXPECT_DEATH(
        (void)original_file.GetFrameTrackFunctionNamesForModule("/path/to/non/existing/module"),
        "contains");
    EXPECT_DEATH(
        (void)original_file.GetSelectedFunctionNamesForModule("/path/to/non/existing/module"),
        "contains");
    EXPECT_DEATH((void)original_file.GetFrameTrackFunctionHashesForModuleLegacy(
                     "/path/to/non/existing/module"),
                 "Check failed: IsLegacyFileFormat\\(\\)");
    EXPECT_DEATH((void)original_file.GetSelectedFunctionHashesForModuleLegacy(
                     "/path/to/non/existing/module"),
                 "Check failed: IsLegacyFileFormat\\(\\)");

    ASSERT_THAT(original_file.SaveToFile(), orbit_test_utils::HasNoError());
  }

  // Load it
  {
    auto preset_file_or_error = orbit_preset_file::ReadPresetFromFile(temporary_file.file_path());
    ASSERT_THAT(preset_file_or_error, orbit_test_utils::HasNoError());
    auto& preset_file = preset_file_or_error.value();

    EXPECT_FALSE(preset_file.IsLegacyFileFormat());
    auto module_paths = preset_file.GetModulePaths();
    ASSERT_EQ(module_paths.size(), 2);

    {
      auto function_names = preset_file.GetSelectedFunctionNamesForModule("/path/to/module1");
      ASSERT_EQ(function_names.size(), 2);
      EXPECT_EQ(function_names.at(0), "f1_1");
      EXPECT_EQ(function_names.at(1), "f1_2");
    }

    {
      auto function_names = preset_file.GetFrameTrackFunctionNamesForModule("/path/to/module1");
      ASSERT_EQ(function_names.size(), 3);
      EXPECT_EQ(function_names.at(0), "ft1_1");
      EXPECT_EQ(function_names.at(1), "ft1_2");
      EXPECT_EQ(function_names.at(2), "ft1_3");
    }

    {
      auto function_names = preset_file.GetSelectedFunctionNamesForModule("/path/to/module2");
      ASSERT_EQ(function_names.size(), 2);
      EXPECT_EQ(function_names.at(0), "f2_1");
      EXPECT_EQ(function_names.at(1), "f2_2");
    }

    {
      auto function_names = preset_file.GetFrameTrackFunctionNamesForModule("/path/to/module2");
      ASSERT_EQ(function_names.size(), 3);
      EXPECT_EQ(function_names.at(0), "ft2_1");
      EXPECT_EQ(function_names.at(1), "ft2_2");
      EXPECT_EQ(function_names.at(2), "ft2_3");
    }

    EXPECT_DEATH(
        (void)preset_file.GetFrameTrackFunctionNamesForModule("/path/to/non/existing/module"),
        "contains");
    EXPECT_DEATH(
        (void)preset_file.GetSelectedFunctionNamesForModule("/path/to/non/existing/module"),
        "contains");
    EXPECT_DEATH((void)preset_file.GetFrameTrackFunctionHashesForModuleLegacy(
                     "/path/to/non/existing/module"),
                 "Check failed: IsLegacyFileFormat\\(\\)");
    EXPECT_DEATH(
        (void)preset_file.GetSelectedFunctionHashesForModuleLegacy("/path/to/non/existing/module"),
        "Check failed: IsLegacyFileFormat\\(\\)");
  }
}

TEST(PresetFile, LegacyFormat) {
  auto temporary_file = CreateTemporaryFileOrDie();
  PresetInfoLegacy info;
  PresetModuleLegacy module;
  module.add_function_hashes(1);
  module.add_function_hashes(2);
  module.add_frame_track_function_hashes(3);
  (*info.mutable_path_to_module())["/path/to/module"] = module;

  PresetFile original_file{temporary_file.file_path(), info};
  EXPECT_TRUE(original_file.IsLegacyFileFormat());
  auto module_paths = original_file.GetModulePaths();
  ASSERT_EQ(module_paths.size(), 1);

  {
    auto function_hashes =
        original_file.GetSelectedFunctionHashesForModuleLegacy("/path/to/module");
    ASSERT_EQ(function_hashes.size(), 2);
    EXPECT_EQ(function_hashes.at(0), 1);
    EXPECT_EQ(function_hashes.at(1), 2);
  }

  {
    auto function_hashes =
        original_file.GetFrameTrackFunctionHashesForModuleLegacy("/path/to/module");
    ASSERT_EQ(function_hashes.size(), 1);
    EXPECT_EQ(function_hashes.at(0), 3);
  }

  EXPECT_DEATH((void)original_file.GetFrameTrackFunctionHashesForModuleLegacy(
                   "/path/to/non/existing/module"),
               "contains");
  EXPECT_DEATH(
      (void)original_file.GetSelectedFunctionHashesForModuleLegacy("/path/to/non/existing/module"),
      "contains");
  EXPECT_DEATH(
      (void)original_file.GetFrameTrackFunctionNamesForModule("/path/to/non/existing/module"),
      "Check failed: \\!IsLegacyFileFormat\\(\\)");
  EXPECT_DEATH(
      (void)original_file.GetSelectedFunctionNamesForModule("/path/to/non/existing/module"),
      "Check failed: \\!IsLegacyFileFormat\\(\\)");

  EXPECT_DEATH((void)original_file.SaveToFile(), "");
}

}  // namespace orbit_preset_file