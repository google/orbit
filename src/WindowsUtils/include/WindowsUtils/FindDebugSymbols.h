// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_FIND_DEBUG_SYMBOLS_H_
#define WINDOWS_UTILS_FIND_DEBUG_SYMBOLS_H_

#include <filesystem>

#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

[[nodiscard]] ErrorMessageOr<std::filesystem::path> FindDebugSymbols(
    std::filesystem::path module_path,
    std::vector<std::filesystem::path> additional_search_directories);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_FIND_DEBUG_SYMBOLS_H_
