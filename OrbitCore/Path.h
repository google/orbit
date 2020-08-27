// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Path {
[[nodiscard]] std::string GetExecutablePath();
[[nodiscard]] std::string GetExecutableDir();
[[nodiscard]] std::string GetFileMappingFileName();
[[nodiscard]] std::string GetSymbolsFileName();
[[nodiscard]] std::string CreateOrGetCacheDir();
[[nodiscard]] std::string CreateOrGetPresetDir();
[[nodiscard]] std::string CreateOrGetCaptureDir();
[[nodiscard]] std::string CreateOrGetDumpDir();
[[nodiscard]] std::string CreateOrGetOrbitAppDataDir();
[[nodiscard]] std::string GetLogFilePathAndCreateDir();
[[nodiscard]] std::string GetIconsPath();
[[nodiscard]] std::string GetFileName(const std::string& file_path);
[[nodiscard]] std::string StripExtension(const std::string& file_path);
[[nodiscard]] std::string GetExtension(const std::string& file_path);
[[nodiscard]] std::string GetDirectory(const std::string& any_path);
[[nodiscard]] std::string GetParentDirectory(std::string any_path);
[[nodiscard]] std::string JoinPath(const std::vector<std::string>& parts);

[[nodiscard]] std::vector<std::string> ListFiles(
    const std::string& directory, const std::function<bool(const std::string&)>& filter =
                                      [](const std::string&) { return true; });
[[nodiscard]] std::vector<std::string> ListFiles(const std::string& directory,
                                                 const std::string& filter);
};  // namespace Path
