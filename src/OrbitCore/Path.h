// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <filesystem>

namespace orbit_core {
[[nodiscard]] std::filesystem::path GetFileMappingFileName();
[[nodiscard]] std::filesystem::path GetSymbolsFileName();
[[nodiscard]] std::filesystem::path CreateOrGetCacheDir();
[[nodiscard]] std::filesystem::path CreateOrGetPresetDir();
[[nodiscard]] std::filesystem::path CreateOrGetCaptureDir();
[[nodiscard]] std::filesystem::path CreateOrGetDumpDir();
[[nodiscard]] std::filesystem::path CreateOrGetOrbitAppDataDir();
[[nodiscard]] std::filesystem::path CreateOrGetLogDir();
[[nodiscard]] std::filesystem::path GetLogFilePath();
};  // namespace orbit_core
