// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientSymbols/QSettingsBasedStorageManager.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stddef.h>

#include <QSettings>
#include <QVariant>
#include <algorithm>
#include <filesystem>

constexpr const char* kSymbolPathsSettingsKey = "symbol_directories";
constexpr const char* kDirectoryPathKey = "directory_path";
constexpr const char* kModuleSymbolFileMappingKey = "module_symbol_file_mapping_key";
constexpr const char* kModuleSymbolFileMappingModuleKey = "module_symbol_file_mapping_module_key";
constexpr const char* kModuleSymbolFileMappingSymbolFileKey =
    "module_symbol_file_mapping_symbol_file_key";
constexpr const char* kDisabledModulesKey = "disabled_modules_key";
constexpr const char* kDisabledModuleKey = "disabled_module_key";
constexpr const char* kEnableStadiaSymbolStoreKey = "enable_stadia_symbol_store_key";
constexpr const char* kEnableMicrosoftSymbolServerKey = "enable_microsoft_symbol_server_key";

namespace orbit_client_symbols {

std::vector<std::filesystem::path> QSettingsBasedStorageManager::LoadPaths() {
  const int size = settings_.beginReadArray(kSymbolPathsSettingsKey);
  std::vector<std::filesystem::path> paths;
  paths.reserve(size);
  for (int i = 0; i < size; ++i) {
    settings_.setArrayIndex(i);
    paths.emplace_back(settings_.value(kDirectoryPathKey).toString().toStdString());
  }
  settings_.endArray();
  return paths;
}

void QSettingsBasedStorageManager::SavePaths(absl::Span<const std::filesystem::path> paths) {
  settings_.beginWriteArray(kSymbolPathsSettingsKey, static_cast<int>(paths.size()));
  for (size_t i = 0; i < paths.size(); ++i) {
    settings_.setArrayIndex(static_cast<int>(i));
    settings_.setValue(kDirectoryPathKey, QString::fromStdString(paths[i].string()));
  }
  settings_.endArray();
}

void QSettingsBasedStorageManager::SaveModuleSymbolFileMappings(
    const ModuleSymbolFileMappings& mappings) {
  settings_.beginWriteArray(kModuleSymbolFileMappingKey, static_cast<int>(mappings.size()));
  int index = 0;
  for (const auto& [module_path, symbol_file_path] : mappings) {
    settings_.setArrayIndex(index);
    settings_.setValue(kModuleSymbolFileMappingModuleKey, QString::fromStdString(module_path));
    settings_.setValue(kModuleSymbolFileMappingSymbolFileKey,
                       QString::fromStdString(symbol_file_path.string()));
    ++index;
  }
  settings_.endArray();
}

ModuleSymbolFileMappings QSettingsBasedStorageManager::LoadModuleSymbolFileMappings() {
  const int size = settings_.beginReadArray(kModuleSymbolFileMappingKey);
  ModuleSymbolFileMappings mappings;
  mappings.reserve(size);
  for (int i = 0; i < size; ++i) {
    settings_.setArrayIndex(i);

    std::string module_path =
        settings_.value(kModuleSymbolFileMappingModuleKey).toString().toStdString();
    std::filesystem::path symbol_file_path = std::filesystem::path{
        settings_.value(kModuleSymbolFileMappingSymbolFileKey).toString().toStdString()};
    mappings[module_path] = symbol_file_path;
  }
  settings_.endArray();
  return mappings;
}

void QSettingsBasedStorageManager::SaveDisabledModulePaths(
    const absl::flat_hash_set<std::string>& paths) {
  settings_.beginWriteArray(kDisabledModulesKey, static_cast<int>(paths.size()));
  int index = 0;
  for (const auto& path : paths) {
    settings_.setArrayIndex(index);
    settings_.setValue(kDisabledModuleKey, QString::fromStdString(path));
    index++;
  }
  settings_.endArray();
}

absl::flat_hash_set<std::string> QSettingsBasedStorageManager::LoadDisabledModulePaths() {
  const int size = settings_.beginReadArray(kDisabledModulesKey);
  absl::flat_hash_set<std::string> paths;
  paths.reserve(size);
  for (int i = 0; i < size; ++i) {
    settings_.setArrayIndex(i);
    paths.insert(settings_.value(kDisabledModuleKey).toString().toStdString());
  }
  settings_.endArray();
  return paths;
}

void QSettingsBasedStorageManager::SaveEnableStadiaSymbolStore(bool enable_stadia_symbol_store) {
  settings_.setValue(kEnableStadiaSymbolStoreKey, enable_stadia_symbol_store);
}

bool QSettingsBasedStorageManager::LoadEnableStadiaSymbolStore() {
  return settings_.value(kEnableStadiaSymbolStoreKey, false).toBool();
}

void QSettingsBasedStorageManager::SaveEnableMicrosoftSymbolServer(
    bool enable_microsoft_symbol_server) {
  settings_.setValue(kEnableMicrosoftSymbolServerKey, enable_microsoft_symbol_server);
}

bool QSettingsBasedStorageManager::LoadEnableMicrosoftSymbolServer() {
  return settings_.value(kEnableMicrosoftSymbolServerKey, false).toBool();
}

}  // namespace orbit_client_symbols