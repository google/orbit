// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H
#define SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H

#include <filesystem>
#include <vector>

namespace orbit_symbol_paths {

// This is a simple wrapper around QSettings, which makes this thread safe, see
// https://doc.qt.io/qt-5/qsettings.html
void SavePaths(const std::vector<std::filesystem::path>& paths);

// This is a simple wrapper around QSettings, which makes this thread safe, see
// https://doc.qt.io/qt-5/qsettings.html
[[nodiscard]] std::vector<std::filesystem::path> LoadPaths();

}  // namespace orbit_symbol_paths

#endif  // SYMBOL_PATHS_PERSISTENT_STORAGE_MANAGER_H