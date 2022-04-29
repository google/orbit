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
  std::unique_ptr<PathConverter> path_converter = PathConverter::Create();
  const absl::flat_hash_map<std::string, VolumeInfo>& device_to_volume_info_map =
      path_converter->GetDeviceToVolumeInfoMap();
  ASSERT_FALSE(device_to_volume_info_map.empty());
}

TEST(PathConverter, ContainsCurrentDrive) {
  std::unique_ptr<PathConverter> path_converter = PathConverter::Create();
  const absl::flat_hash_map<std::string, VolumeInfo>& device_to_volume_info_map =
      path_converter->GetDeviceToVolumeInfoMap();

  std::string current_drive = orbit_base::GetExecutablePath().root_name().string() + "\\";

  for (const auto& [device, volume_info] : device_to_volume_info_map) {
    for (const std::string& path : volume_info.paths) {
      if (path == current_drive) {
        return;
      }
    }
  }

  FAIL() << "device_to_volume_info_map does not contain current drive.\n"
         << path_converter->ToString();
}

}  // namespace orbit_windows_utils
