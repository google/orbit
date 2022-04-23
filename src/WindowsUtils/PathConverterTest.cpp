// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/PathConverter.h"
namespace orbit_windows_utils {

using orbit_test_utils::HasValue;
using orbit_windows_utils::PathConverter;

TEST(PathConverter, ListDevices) {
  ErrorMessageOr<std::unique_ptr<PathConverter>> converter_or_error = PathConverter::Create();
  EXPECT_THAT(converter_or_error, HasValue());
  const std::unique_ptr<PathConverter>& converter = converter_or_error.value();

  const absl::flat_hash_map<std::string, VolumeInfo>& device_to_volume_info_map =
      converter->GetDeviceToVolumeInfoMap();

  for (const auto& [device, volume_info] : device_to_volume_info_map) {
    std::string paths;
    for (const std::string& path : volume_info.paths) {
      paths += path + " ";
    }
    ORBIT_LOG("device: %s volume: %s paths: %s", device, volume_info.volume_name, paths);
  }
}

TEST(PathConverter, ContainsCurrentDrive) {
  ErrorMessageOr<std::unique_ptr<PathConverter>> converter_or_error = PathConverter::Create();
  EXPECT_THAT(converter_or_error, HasValue());
  const std::unique_ptr<PathConverter>& converter = converter_or_error.value();
  const absl::flat_hash_map<std::string, VolumeInfo>& device_to_volume_info_map =
      converter->GetDeviceToVolumeInfoMap();

  std::string current_drive = orbit_base::GetExecutablePath().root_name().string() + "\\";
  bool found_root_path = false;

  for (const auto& [device, volume_info] : device_to_volume_info_map) {
    std::string paths;
    for (const std::string& path : volume_info.paths) {
      if (path == current_drive) {
        return;
      }
    }
  }

  ASSERT_TRUE(found_root_path);
}

}  // namespace orbit_windows_utils
