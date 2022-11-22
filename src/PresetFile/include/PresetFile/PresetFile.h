// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PRESET_FILE_PRESET_FILE_H_
#define PRESET_FILE_PRESET_FILE_H_

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "ClientProtos/preset.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_preset_file {

class PresetFile final {
 public:
  explicit PresetFile(std::filesystem::path file_path, orbit_client_protos::PresetInfo preset_info)
      : file_path_{std::move(file_path)},
        is_legacy_format_{false},
        preset_info_{std::move(preset_info)} {}

  explicit PresetFile(std::filesystem::path file_path,
                      orbit_client_protos::PresetInfoLegacy preset_info_legacy)
      : file_path_{std::move(file_path)},
        is_legacy_format_{true},
        preset_info_legacy_{std::move(preset_info_legacy)} {}

  [[nodiscard]] const std::filesystem::path& file_path() const { return file_path_; }

  [[nodiscard]] std::vector<std::filesystem::path> GetModulePaths() const;
  [[nodiscard]] size_t GetNumberOfFunctionsForModule(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] bool IsLegacyFileFormat() const;

  void SetIsLoaded(bool is_loaded) { is_loaded_ = is_loaded; }
  [[nodiscard]] bool IsLoaded() const { return is_loaded_; }

  [[nodiscard]] std::vector<uint64_t> GetSelectedFunctionHashesForModuleLegacy(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] std::vector<uint64_t> GetFrameTrackFunctionHashesForModuleLegacy(
      const std::filesystem::path& module_path) const;

  [[nodiscard]] std::vector<std::string> GetSelectedFunctionNamesForModule(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] std::vector<std::string> GetFrameTrackFunctionNamesForModule(
      const std::filesystem::path& module_path) const;

  ErrorMessageOr<void> SaveToFile() const;

 private:
  std::filesystem::path file_path_;
  bool is_legacy_format_;
  bool is_loaded_ = false;
  orbit_client_protos::PresetInfo preset_info_;
  orbit_client_protos::PresetInfoLegacy preset_info_legacy_;
};

ErrorMessageOr<PresetFile> ReadPresetFromFile(const std::filesystem::path& file_path);

}  // namespace orbit_preset_file

#endif  // PRESET_FILE_PRESET_FILE_H_
