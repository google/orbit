// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H
#define SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H

#include <absl/container/flat_hash_map.h>
#include <absl/types/span.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace orbit_symbol_paths {

class PersistentStorageManager {
 public:
  virtual ~PersistentStorageManager() = default;

  virtual void SavePaths(absl::Span<const std::filesystem::path> paths) = 0;
  [[nodiscard]] virtual std::vector<std::filesystem::path> LoadPaths() = 0;

  // The hash map uses the key std::string for the module path instead of std::filesystem::path,
  // because the module path is always linux path (from the instance). When this is compiled on
  // windows, std::filesystem::path will use backslash instead of slash as directory separator which
  // leads to confusion.
  virtual void SaveModuleSymbolFileMappings(
      const absl::flat_hash_map<std::string, std::filesystem::path>& mappings) = 0;
  [[nodiscard]] virtual absl::flat_hash_map<std::string, std::filesystem::path>
  LoadModuleSymbolFileMappings() = 0;
};

}  // namespace orbit_symbol_paths

#endif  // SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H