// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H
#define SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H

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
};

}  // namespace orbit_symbol_paths

#endif  // SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H