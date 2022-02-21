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
  [[nodiscard]] virtual std::vector<std::filesystem::path> LoadPaths() override;
};

}  // namespace orbit_symbol_paths

#endif  // SYMBOL_PATHS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_