// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PATHS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_
#define SYMBOL_PATHS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_

#include "SymbolPaths/PersistentStorageManager.h"

namespace orbit_symbol_paths {

class QSettingsBasedStorageManager : public PersistentStorageManager {
 public:
  void SavePaths(absl::Span<const std::filesystem::path> paths) override;
  [[nodiscard]] std::vector<std::filesystem::path> LoadPaths() override;
  void SaveModuleSymbolFileMappings(
      const absl::flat_hash_map<std::string, std::filesystem::path>& mappings) override;
  [[nodiscard]] absl::flat_hash_map<std::string, std::filesystem::path>
  LoadModuleSymbolFileMappings() override;
};

}  // namespace orbit_symbol_paths

#endif  // SYMBOL_PATHS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_