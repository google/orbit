// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PresetFile/PresetFile.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/text_format.h>
#include <string.h>

#include <algorithm>
#include <string_view>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_preset_file {

using orbit_client_protos::PresetInfo;
using orbit_client_protos::PresetInfoLegacy;

namespace {

constexpr const char* kPresetFileSignature = "# ORBIT preset file\n";

ErrorMessageOr<PresetInfo> ReadPresetFromString(std::string_view content) {
  google::protobuf::io::ArrayInputStream is{content.data(), static_cast<int>(content.size())};
  PresetInfo preset_info;
  if (!google::protobuf::TextFormat::Parse(&is, &preset_info)) {
    return ErrorMessage{"Unable to parse message"};
  }

  return preset_info;
}

ErrorMessageOr<PresetInfoLegacy> ReadLegacyPresetFromString(std::string_view content) {
  PresetInfoLegacy preset_info;
  if (!preset_info.ParseFromArray(content.data(), content.size())) {
    return ErrorMessage{"Unable to parse message"};
  }

  return preset_info;
}

}  // namespace

[[nodiscard]] static std::vector<std::filesystem::path> GetPresetModulePathsLegacy(
    const orbit_client_protos::PresetInfoLegacy& preset_info) {
  std::vector<std::filesystem::path> module_paths;
  for (const auto& entry : preset_info.path_to_module()) {
    module_paths.emplace_back(entry.first);
  }

  return module_paths;
}

std::vector<std::filesystem::path> PresetFile::GetModulePaths() const {
  if (IsLegacyFileFormat()) {
    return GetPresetModulePathsLegacy(preset_info_legacy_);
  }

  std::vector<std::filesystem::path> module_paths;
  for (const auto& entry : preset_info_.modules()) {
    module_paths.emplace_back(entry.first);
  }

  return module_paths;
}

size_t PresetFile::GetNumberOfFunctionsForModule(const std::filesystem::path& module_path) const {
  if (IsLegacyFileFormat()) {
    return preset_info_legacy_.path_to_module().at(module_path.string()).function_hashes_size();
  }

  return preset_info_.modules().at(module_path.string()).function_names_size();
}

bool PresetFile::IsLegacyFileFormat() const { return is_legacy_format_; }

std::vector<uint64_t> PresetFile::GetSelectedFunctionHashesForModuleLegacy(
    const std::filesystem::path& module_path) const {
  ORBIT_CHECK(IsLegacyFileFormat());
  ORBIT_CHECK(preset_info_legacy_.path_to_module().contains(module_path.string()));
  const auto& function_hashes =
      preset_info_legacy_.path_to_module().at(module_path.string()).function_hashes();
  return {function_hashes.begin(), function_hashes.end()};
}

std::vector<uint64_t> PresetFile::GetFrameTrackFunctionHashesForModuleLegacy(
    const std::filesystem::path& module_path) const {
  ORBIT_CHECK(IsLegacyFileFormat());
  ORBIT_CHECK(preset_info_legacy_.path_to_module().contains(module_path.string()));
  const auto& frame_track_function_hashes =
      preset_info_legacy_.path_to_module().at(module_path.string()).frame_track_function_hashes();
  return {frame_track_function_hashes.begin(), frame_track_function_hashes.end()};
}

std::vector<std::string> PresetFile::GetSelectedFunctionNamesForModule(
    const std::filesystem::path& module_path) const {
  ORBIT_CHECK(!IsLegacyFileFormat());
  ORBIT_CHECK(preset_info_.modules().contains(module_path.string()));
  const auto& function_names = preset_info_.modules().at(module_path.string()).function_names();
  return {function_names.begin(), function_names.end()};
}

std::vector<std::string> PresetFile::GetFrameTrackFunctionNamesForModule(
    const std::filesystem::path& module_path) const {
  ORBIT_CHECK(!IsLegacyFileFormat());
  ORBIT_CHECK(preset_info_.modules().contains(module_path.string()));
  const auto& function_names =
      preset_info_.modules().at(module_path.string()).frame_track_function_names();
  return {function_names.begin(), function_names.end()};
}

ErrorMessageOr<PresetFile> ReadPresetFromFile(const std::filesystem::path& file_path) {
  OUTCOME_TRY(auto&& file_content, orbit_base::ReadFileToString(file_path));

  // If signature is not detected assume the file is in old format.
  if (!absl::StartsWith(file_content, kPresetFileSignature)) {
    OUTCOME_TRY(auto&& preset_info_legacy, ReadLegacyPresetFromString(file_content));
    return PresetFile{file_path, preset_info_legacy};
  }

  OUTCOME_TRY(auto&& preset_info,
              ReadPresetFromString(file_content.c_str() + strlen(kPresetFileSignature)));
  return PresetFile{file_path, preset_info};
}

ErrorMessageOr<void> PresetFile::SaveToFile() const {
  ORBIT_CHECK(!is_legacy_format_);

  OUTCOME_TRY(auto&& fd, orbit_base::OpenFileForWriting(file_path_));
  ORBIT_LOG("Saving preset to \"%s\"", file_path_.string());

  auto write_result = orbit_base::WriteFully(fd, kPresetFileSignature);
  if (write_result.has_error()) {
    std::string error_message = absl::StrFormat(
        "Failed to save preset to \"%s\": %s", file_path_.string(), write_result.error().message());
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }

  std::string content;
  if (!google::protobuf::TextFormat::PrintToString(preset_info_, &content)) {
    return ErrorMessage{"Unable to convert message to string"};
  }

  write_result = orbit_base::WriteFully(fd, content);
  if (write_result.has_error()) {
    std::string error_message = absl::StrFormat(
        "Failed to save preset to \"%s\": %s", file_path_.string(), write_result.error().message());
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }

  return outcome::success();
}

}  // namespace orbit_preset_file
