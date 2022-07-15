// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PATHS_PATHS_H_
#define ORBIT_PATHS_PATHS_H_

#include <filesystem>

#include "OrbitBase/Result.h"

namespace orbit_paths {

// TODO(b/238986831) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetSymbolsFilePathUnsafe();
ErrorMessageOr<std::filesystem::path> GetSymbolsFilePath();

// TODO(b/238986334) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetCacheDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetCacheDir();

// TODO(b/238986330) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetPresetDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetPresetDir();

// TODO(b/239014904) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetCaptureDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetCaptureDir();

// TODO(b/238986372) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetDumpDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetDumpDir();

// TODO(b/238986370) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetOrbitAppDataDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitAppDataDir();

// TODO(b/238985362) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetOrbitUserDataDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitUserDataDir();

// TODO(b/238986342) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path CreateOrGetLogDirUnsafe();
ErrorMessageOr<std::filesystem::path> CreateOrGetLogDir();

// TODO(b/238986108) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetLogFilePathUnsafe();
ErrorMessageOr<std::filesystem::path> GetLogFilePath();

// TODO(b/238986346) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetPresetDirPriorTo1_66Unsafe();
ErrorMessageOr<std::filesystem::path> GetPresetDirPriorTo1_66();

// TODO(b/238986348) Replace deprecated and unsafe function with safe alternative.
[[nodiscard, deprecated]] std::filesystem::path GetCaptureDirPriorTo1_66Unsafe();
ErrorMessageOr<std::filesystem::path> GetCaptureDirPriorTo1_66();

}  // namespace orbit_paths

#endif  // ORBIT_PATHS_PATHS_H_
