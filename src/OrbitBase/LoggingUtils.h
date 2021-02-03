// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_LOGGING_UTILS_H_
#define ORBIT_BASE_LOGGING_UTILS_H_

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <filesystem>

#include "OrbitBase/Result.h"

constexpr const char* kLogFileNameTimeFormat = "%Y_%m_%d_%H_%M_%S";
constexpr absl::string_view kLogFileNameDelimiter = "Orbit-%s-%u.log";

[[nodiscard]] std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& log_dir);
ErrorMessageOr<absl::Duration> GetLogFileLifetime(const std::string& log_file_name);
[[nodiscard]] std::vector<std::filesystem::path> FilterOldLogFiles(
    const std::vector<std::filesystem::path>& log_file_paths);
// This function tries to remove files even when an error is returned. If some files are unable to
// remove, it returns an error message to record names of those functions and details about the
// remove failures.
ErrorMessageOr<void> RemoveFiles(const std::vector<std::filesystem::path>& log_file_paths);

#endif  // ORBIT_BASE_LOGGING_UTILS_H_