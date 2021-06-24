// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PATHS_PATHS_H_
#define ORBIT_PATHS_PATHS_H_

#include <filesystem>

namespace orbit_paths {

[[nodiscard]] std::filesystem::path GetSymbolsFilePath();
[[nodiscard]] std::filesystem::path CreateOrGetCacheDir();
[[nodiscard]] std::filesystem::path CreateOrGetPresetDir();
[[nodiscard]] std::filesystem::path CreateOrGetCaptureDir();
[[nodiscard]] std::filesystem::path CreateOrGetDumpDir();
[[nodiscard]] std::filesystem::path CreateOrGetOrbitAppDataDir();
[[nodiscard]] std::filesystem::path CreateOrGetOrbitUserDataDir();
[[nodiscard]] std::filesystem::path CreateOrGetLogDir();
[[nodiscard]] std::filesystem::path GetLogFilePath();

[[nodiscard]] std::filesystem::path GetPresetDirPriorTo1_66();
[[nodiscard]] std::filesystem::path GetCaptureDirPriorTo1_66();

}  // namespace orbit_paths

#endif  // ORBIT_PATHS_PATHS_H_
