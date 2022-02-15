// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QSettings>
#include <filesystem>

#include "SymbolPaths/PersistentStorageManager.h"

constexpr const char* kSymbolPathsSettingsKey = "symbol_directories";
constexpr const char* kDirectoryPathKey = "directory_path";

namespace orbit_symbol_paths {

std::vector<std::filesystem::path> LoadPaths() {
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

void SavePaths(const std::vector<std::filesystem::path>& paths) {
  QSettings settings{};
  settings.beginWriteArray(kSymbolPathsSettingsKey, static_cast<int>(paths.size()));
  for (size_t i = 0; i < paths.size(); ++i) {
    settings.setArrayIndex(static_cast<int>(i));
    settings.setValue(kDirectoryPathKey, QString::fromStdString(paths[i].string()));
  }
  settings.endArray();
}

}  // namespace orbit_symbol_paths