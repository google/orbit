// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESET_FILE_H_
#define ORBIT_GL_PRESET_FILE_H_

#include <filesystem>
#include <vector>

#include "preset.pb.h"

namespace orbit_gl {

class PresetFile final {
 public:
  explicit PresetFile(std::filesystem::path file_path, orbit_client_protos::PresetInfo preset_info)
      : file_path_{std::move(file_path)}, preset_info_{std::move(preset_info)} {}

  [[nodiscard]] const std::filesystem::path& file_path() const { return file_path_; }

  [[nodiscard]] std::vector<std::filesystem::path> GetModulePaths() const;
  [[nodiscard]] size_t GetNumberOfFunctionsForModule(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] bool IsLegacyFileFormat() const;

  [[nodiscard]] std::vector<uint64_t> GetSelectedFunctionHashesForModuleLegacy(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] std::vector<uint64_t> GetFrameTrackFunctionHashesForModuleLegacy(
      const std::filesystem::path& module_path) const;

  [[nodiscard]] std::vector<std::string> GetSelectedFunctionNamesForModule(
      const std::filesystem::path& module_path) const;
  [[nodiscard]] std::vector<std::string> GetFrameTrackFunctionNamesForModule(
      const std::filesystem::path& module_path) const;

 private:
  std::filesystem::path file_path_;
  orbit_client_protos::PresetInfo preset_info_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PRESET_FILE_H_
