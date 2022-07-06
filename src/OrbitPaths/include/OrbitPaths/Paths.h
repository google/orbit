// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PATHS_PATHS_H_
#define ORBIT_PATHS_PATHS_H_

#include <filesystem>

#include "OrbitBase/Result.h"

namespace orbit_paths {

// TODO(b/238986831) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetSymbolsFilePath();
ErrorMessageOr<std::filesystem::path> GetSymbolsFilePathSafe();

// TODO(b/238986334) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetCacheDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetCacheDirSafe();

// TODO(b/238986330) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetPresetDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetPresetDirSafe();

// TODO(b/239014904) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetCaptureDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetCaptureDirSafe();

// TODO(b/238986372) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetDumpDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetDumpDirSafe();

// TODO(b/238986370) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetOrbitAppDataDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitAppDataDirSafe();

// TODO(b/238985362) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetOrbitUserDataDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitUserDataDirSafe();

// TODO(b/238986342) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetLogDir();
ErrorMessageOr<std::filesystem::path> CreateOrGetLogDirSafe();

// TODO(b/238986108) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetLogFilePath();
ErrorMessageOr<std::filesystem::path> GetLogFilePathSafe();

[[nodiscard]] std::filesystem::path GetPresetDirPriorTo1_66();
[[nodiscard]] std::filesystem::path GetCaptureDirPriorTo1_66();

}  // namespace orbit_paths

#endif  // ORBIT_PATHS_PATHS_H_
