// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolPaths/QSettingsBasedStorageManager.h"

#include <QSettings>
#include <filesystem>
#include <memory>

constexpr const char* kSymbolPathsSettingsKey = "symbol_directories";
constexpr const char* kDirectoryPathKey = "directory_path";
constexpr const char* kModuleSymbolFileMappingKey = "module_symbol_file_mapping_key";
constexpr const char* kModuleSymbolFileMappingModuleKey = "module_symbol_file_mapping_module_key";
constexpr const char* kModuleSymbolFileMappingSymbolFileKey =
    "module_symbol_file_mapping_symbol_file_key";

namespace orbit_symbol_paths {

std::vector<std::filesystem::path> QSettingsBasedStorageManager::LoadPaths() {
  QSettings settings{};
  const int size = settings.beginReadArray(kSymbolPathsSettingsKey);
  std::vector<std::filesystem::path> paths{};
  paths.reserve(size);
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    paths.emplace_back(settings.value(kDirectoryPathKey).toString().toStdString());
  }
  settings.endArray();
  return paths;
}

void QSettingsBasedStorageManager::SavePaths(absl::Span<const std::filesystem::path> paths) {
  QSettings settings{};
  settings.beginWriteArray(kSymbolPathsSettingsKey, static_cast<int>(paths.size()));
  for (size_t i = 0; i < paths.size(); ++i) {
    settings.setArrayIndex(static_cast<int>(i));
    settings.setValue(kDirectoryPathKey, QString::fromStdString(paths[i].string()));
  }
  settings.endArray();
}

void QSettingsBasedStorageManager::SaveModuleSymbolFileMappings(
    const absl::flat_hash_map<std::string, std::filesystem::path>& mappings) {
  QSettings settings{};
  settings.beginWriteArray(kModuleSymbolFileMappingKey, static_cast<int>(mappings.size()));
  int index = 0;
  for (const auto& [module_path, symbol_file_path] : mappings) {
    settings.setArrayIndex(index);
    settings.setValue(kModuleSymbolFileMappingModuleKey, QString::fromStdString(module_path));
    settings.setValue(kModuleSymbolFileMappingSymbolFileKey,
                      QString::fromStdString(symbol_file_path.string()));
    ++index;
  }
  settings.endArray();
}

[[nodiscard]] absl::flat_hash_map<std::string, std::filesystem::path>
QSettingsBasedStorageManager::LoadModuleSymbolFileMappings() {
  QSettings settings{};
  const int size = settings.beginReadArray(kModuleSymbolFileMappingKey);
  absl::flat_hash_map<std::string, std::filesystem::path> mappings{};
  mappings.reserve(size);
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);

    std::string module_path =
        settings.value(kModuleSymbolFileMappingModuleKey).toString().toStdString();
    std::filesystem::path symbol_file_path = std::filesystem::path{
        settings.value(kModuleSymbolFileMappingSymbolFileKey).toString().toStdString()};
    mappings[module_path] = symbol_file_path;
  }
  settings.endArray();
  return mappings;
}

}  // namespace orbit_symbol_paths