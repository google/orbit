// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LoggingUtils.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <system_error>

#include "OrbitBase/Logging.h"

const absl::Duration kLogFileLifetime = absl::Hours(24 * 7);  // one week
// Determined by kLogFileNameDelimiter, i.e., the format of log file names.
const int kTimestampStartPos = 6;
// Determined by kLogFileNameTimeFormat, i.e., the format of timestamp contained in log file names.
const int kTimestampStringLength = 19;

std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& log_dir) {
  std::vector<std::filesystem::path> files_in_dir;
  for (const auto& file_path : std::filesystem::recursive_directory_iterator(log_dir)) {
    if (file_path.is_regular_file()) {
      files_in_dir.push_back(file_path.path());
    }
  }
  return files_in_dir;
}

ErrorMessageOr<absl::Duration> GetLogFileLifetime(const std::string& log_file_name) {
  if (log_file_name.size() < kTimestampStartPos + kTimestampStringLength) {
    return ErrorMessage(
        absl::StrFormat("Unable to extract time information from log file: %s", log_file_name));
  }
  std::string timestamp_string = log_file_name.substr(kTimestampStartPos, kTimestampStringLength);
  absl::Time log_file_timestamp;
  std::string parse_time_error;
  if (!absl::ParseTime(kLogFileNameTimeFormat, timestamp_string, absl::LocalTimeZone(),
                       &log_file_timestamp, &parse_time_error)) {
    return ErrorMessage(
        absl::StrFormat("Error while parsing time information from log file %s : %s", log_file_name,
                        parse_time_error));
  }
  return absl::Now() - log_file_timestamp;
}

std::vector<std::filesystem::path> FilterOldLogFiles(
    const std::vector<std::filesystem::path>& log_file_paths) {
  std::vector<std::filesystem::path> old_files;
  for (std::filesystem::path log_file_path : log_file_paths) {
    ErrorMessageOr<absl::Duration> get_lifetime_result =
        GetLogFileLifetime(log_file_path.filename().string());
    if (!get_lifetime_result) {
      LOG("Warning: %s", get_lifetime_result.error().message());
      continue;
    }
    if (get_lifetime_result.value() > kLogFileLifetime) {
      old_files.push_back(log_file_path);
    }
  }
  return old_files;
}

ErrorMessageOr<void> RemoveFiles(const std::vector<std::filesystem::path>& log_file_paths) {
  std::string error_message;
  for (const auto& log_file_path : log_file_paths) {
    std::error_code file_remove_error;
    if (!std::filesystem::remove(log_file_path, file_remove_error)) {
      absl::StrAppend(&error_message, "Error while removing ", log_file_path.filename().string(),
                      ": ", file_remove_error.message(), "\n");
    }
  }
  if (!error_message.empty()) {
    return ErrorMessage(error_message);
  }
  return outcome::success();
}