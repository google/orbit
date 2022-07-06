// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PATHS_PATHS_H_
#define ORBIT_PATHS_PATHS_H_

#include <filesystem>

#include "OrbitBase/Result.h"

namespace orbit_paths {

[[nodiscard]] std::filesystem::path GetSymbolsFilePath();
[[nodiscard]] std::filesystem::path CreateOrGetCacheDir();

// TODO(b/238986330) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetPresetDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetPresetDirSafe();

// TODO(b/239014904) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetCaptureDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetCaptureDirSafe();
[[nodiscard]] std::filesystem::path CreateOrGetDumpDir();
[[nodiscard]] std::filesystem::path CreateOrGetOrbitAppDataDir();

// TODO(b/238985362) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetOrbitUserDataDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitUserDataDirSafe();
[[nodiscard]] std::filesystem::path CreateOrGetLogDir();
[[nodiscard]] std::filesystem::path GetLogFilePath();

[[nodiscard]] std::filesystem::path GetPresetDirPriorTo1_66();
[[nodiscard]] std::filesystem::path GetCaptureDirPriorTo1_66();

}  // namespace orbit_paths

#endif  // ORBIT_PATHS_PATHS_H_
