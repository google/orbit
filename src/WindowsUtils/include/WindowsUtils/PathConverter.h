// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PATH_CONVERTER_H_
#define WINDOWS_UTILS_PATH_CONVERTER_H_

#include <absl/container/flat_hash_map.h>

#include <string>

#include "OrbitBase/Result.h"

namespace orbit_windows_utils {

struct VolumeInfo {
  std::string volume_name;
  std::string device_name;
  std::vector<std::string> paths;
};

struct PathConverter {
  // Transform an input of the form "\Device\HarddiskVolumeN\..." to "C:\...".
  virtual ErrorMessageOr<std::string> DeviceToDrive(std::string_view full_path) = 0;

  // Return a map of device names to VolumeInfo objects.
  virtual const absl::flat_hash_map<std::string, VolumeInfo>& GetDeviceToVolumeInfoMap() = 0;

  // Return a summary of the path converter as string.
  [[nodiscard]] virtual std::string ToString() const = 0;

  // Create a converter.
  static std::unique_ptr<PathConverter> Create();
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_DEVICE_TO_DRIVE_H_
