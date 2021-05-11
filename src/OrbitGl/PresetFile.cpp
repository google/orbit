// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PresetFile.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

[[nodiscard]] static std::vector<std::filesystem::path> GetPresetModulePathsLegacy(
    const orbit_client_protos::PresetInfo& preset_info) {
  std::vector<std::filesystem::path> module_paths;
  for (const auto& entry : preset_info.path_to_module()) {
    module_paths.emplace_back(entry.first);
  }

  return module_paths;
}

std::vector<std::filesystem::path> PresetFile::GetModulePaths() const {
  if (preset_info_.modules().empty()) {
    return GetPresetModulePathsLegacy(preset_info_);
  }

  std::vector<std::filesystem::path> module_paths;
  for (const auto& entry : preset_info_.modules()) {
    module_paths.emplace_back(entry.first);
  }

  return module_paths;
}

size_t PresetFile::GetNumberOfFunctionsForModule(const std::filesystem::path& module_path) const {
  if (preset_info_.modules().empty()) {
    return preset_info_.path_to_module().at(module_path.string()).function_hashes_size();
  }

  return preset_info_.modules().at(module_path.string()).function_names_size();
}

bool PresetFile::IsLegacyFileFormat() const { return preset_info_.modules().empty(); }

std::vector<uint64_t> PresetFile::GetSelectedFunctionHashesForModuleLegacy(
    const std::filesystem::path& module_path) const {
  CHECK(IsLegacyFileFormat());
  const auto& function_hashes =
      preset_info_.path_to_module().at(module_path.string()).function_hashes();
  return {function_hashes.begin(), function_hashes.end()};
}

std::vector<uint64_t> PresetFile::GetFrameTrackFunctionHashesForModuleLegacy(
    const std::filesystem::path& module_path) const {
  CHECK(IsLegacyFileFormat());
  const auto& frame_track_function_hashes =
      preset_info_.path_to_module().at(module_path.string()).frame_track_function_hashes();
  return {frame_track_function_hashes.begin(), frame_track_function_hashes.end()};
}

std::vector<std::string> PresetFile::GetSelectedFunctionNamesForModule(
    const std::filesystem::path& module_path) const {
  CHECK(!IsLegacyFileFormat());
  const auto& function_names = preset_info_.modules().at(module_path.string()).function_names();
  return {function_names.begin(), function_names.end()};
}

std::vector<std::string> PresetFile::GetFrameTrackFunctionNamesForModule(
    const std::filesystem::path& module_path) const {
  CHECK(!IsLegacyFileFormat());
  const auto& function_names =
      preset_info_.modules().at(module_path.string()).frame_track_function_names();
  return {function_names.begin(), function_names.end()};
}

}  // namespace orbit_gl
