// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/PathConverter.h"

#include <Windows.h>
#include <absl/strings/match.h>

#include <cstring>

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

namespace {

using orbit_windows_utils::VolumeInfo;

// https://docs.microsoft.com/en-us/windows/win32/fileio/displaying-volume-paths
std::vector<std::string> GetVolumePaths(const char* volume) {
  DWORD buffer_size = 1024;
  std::string paths(buffer_size, 0);

  // Obtain all of the paths for this volume.
  // From Microsoft's documentation, regarding the passed in buffer:
  // "A pointer to a buffer that receives the list of drive letters and mounted folder paths. The
  // list is an array of null-terminated strings terminated by an additional NULL character. If the
  // buffer is not large enough to hold the complete list, the buffer holds as much of the list as
  // possible."
  BOOL success = GetVolumePathNamesForVolumeNameA(volume, paths.data(), buffer_size, &buffer_size);
  if (!success && GetLastError() == ERROR_MORE_DATA) {
    // The buffer was too small, try again with the new suggested size.
    paths = std::string(buffer_size, 0);
    success = GetVolumePathNamesForVolumeNameA(volume, paths.data(), buffer_size, &buffer_size);
  }

  if (!success) {
    ORBIT_ERROR("Calling GetVolumePathNamesForVolumeNameA: %s", orbit_base::GetLastErrorAsString());
    return {};
  }

  // Return paths as vector.
  std::vector<std::string> path_vector;
  for (const char* path = paths.c_str(); path[0] != '\0'; path += std::strlen(path) + 1) {
    path_vector.push_back(path);
  }
  return path_vector;
}

// Return a map of device names to VolumeInfo objects.
absl::flat_hash_map<std::string, VolumeInfo> BuildDeviceToVolumeInfoMap() {
  absl::flat_hash_map<std::string, VolumeInfo> device_to_volume_info_map;

  // Enumerate all volumes in the system.
  char volume_name[MAX_PATH] = "";
  HANDLE find_handle = FindFirstVolumeA(volume_name, ARRAYSIZE(volume_name));
  if (find_handle == INVALID_HANDLE_VALUE) {
    ORBIT_ERROR("Calling FindFirstVolumeA: %s", orbit_base::GetLastErrorAsString());
    return {};
  }
  orbit_base::unique_resource handle_closer(find_handle, FindVolumeClose);

  while (true) {
    // Validate volume name.
    size_t volume_name_length = std::strlen(volume_name);
    size_t last_index = std::strlen(volume_name) - 1;
    if (volume_name[0] != '\\' || volume_name[1] != '\\' || volume_name[2] != '?' ||
        volume_name[3] != '\\' || volume_name[last_index] != '\\') {
      ORBIT_ERROR("FindFirstVolumeA/FindNextVolumeA returned a bad path: %s", volume_name);
      return {};
    }

    char device_name[MAX_PATH] = "";
    // QueryDosDeviceA does not allow a trailing backslash, so temporarily remove it.
    volume_name[last_index] = '\0';
    // Get device name from volume name, skipping the "\\?\" prefix.
    ORBIT_CHECK(volume_name_length > 4);
    if (QueryDosDeviceA(&volume_name[4], device_name, ARRAYSIZE(device_name)) == 0) {
      ORBIT_ERROR("Calling QueryDosDeviceA: %s", orbit_base::GetLastErrorAsString());
      return {};
    }
    volume_name[last_index] = '\\';

    VolumeInfo volume_info;
    volume_info.device_name = absl::StrFormat("%s%s", device_name, "\\");
    volume_info.volume_name = volume_name;
    volume_info.paths = GetVolumePaths(volume_name);
    device_to_volume_info_map.emplace(volume_info.device_name, volume_info);

    //  Move on to the next volume.
    if (!FindNextVolumeA(find_handle, volume_name, ARRAYSIZE(volume_name))) {
      if (GetLastError() != ERROR_NO_MORE_FILES) {
        ORBIT_ERROR("Calling FindNextVolumeA: %s", orbit_base::GetLastErrorAsString());
        return device_to_volume_info_map;
      }
      break;
    }
  }

  return device_to_volume_info_map;
}

class PathConverterImpl : public orbit_windows_utils::PathConverter {
 public:
  PathConverterImpl() : device_to_volume_info_map_(BuildDeviceToVolumeInfoMap()) {}

  const absl::flat_hash_map<std::string, VolumeInfo>& GetDeviceToVolumeInfoMap() override {
    return device_to_volume_info_map_;
  }

  ErrorMessageOr<std::string> DeviceToDrive(std::string_view full_path) override {
    for (const auto& [device, volume_info] : device_to_volume_info_map_) {
      if (absl::StartsWith(full_path, device) && !volume_info.paths.empty()) {
        std::string suffix(full_path.begin() + device.size(), full_path.end());
        return volume_info.paths[0] + suffix;
      }
    }

    return ErrorMessage(absl::StrFormat("Could not convert path %s\n %s", full_path, ToString()));
  }

  std::string ToString() const override {
    std::string summary =
        absl::StrFormat("PathConverter has %u device names:", device_to_volume_info_map_.size());
    for (const auto& [device, volume_info] : device_to_volume_info_map_) {
      std::string paths;
      for (const std::string& path : volume_info.paths) {
        paths += path + " ";
      }
      summary += absl::StrFormat("device: %s volume: %s paths: %s\n", device,
                                 volume_info.volume_name, paths);
    }
    return summary;
  }

 private:
  absl::flat_hash_map<std::string, VolumeInfo> device_to_volume_info_map_;
};

}  // namespace

namespace orbit_windows_utils {

std::unique_ptr<PathConverter> PathConverter::Create() {
  return std::make_unique<PathConverterImpl>();
}

}  // namespace orbit_windows_utils
