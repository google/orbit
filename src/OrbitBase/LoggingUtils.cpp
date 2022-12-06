// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LoggingUtils.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <absl/types/span.h>

#include <string>
#include <string_view>
#include <system_error>

#include "OrbitBase/Logging.h"

namespace orbit_base_internal {

const absl::Duration kLogFileLifetime = absl::Hours(24 * 7);  // one week
// Determined by kLogFileNameDelimiter, i.e., the format of log file names.
const int kTimestampStartPos = 6;
// Determined by kLogFileNameTimeFormat, i.e., the format of timestamp contained in log file names.
const int kTimestampStringLength = 19;

std::vector<std::filesystem::path> ListFilesRecursivelyIgnoreErrors(
    const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> files_in_dir;
  std::error_code error;
  auto directory_iterator = std::filesystem::recursive_directory_iterator(dir, error);
  if (error) {
    ORBIT_ERROR("Unable to open directory \"%s\": %s", dir.string(), error.message());
    return {};
  }

  for (auto it = std::filesystem::begin(directory_iterator),
            end = std::filesystem::end(directory_iterator);
       it != end; it.increment(error)) {
    if (error) {
      ORBIT_ERROR("directory_iterator::increment failed for \"%s\": %s (stopping)", dir.string(),
                  error.message());
      break;
    }

    bool is_regular_file = it->is_regular_file(error);
    if (error) {
      ORBIT_ERROR("Unable to stat \"%s\": %s (will ignore)", it->path().string(), error.message());
      continue;
    }

    if (is_regular_file) {
      files_in_dir.push_back(it->path());
    }
  }

  return files_in_dir;
}

ErrorMessageOr<absl::Time> ParseLogFileTimestamp(std::string_view log_file_name) {
  if (log_file_name.size() < kTimestampStartPos + kTimestampStringLength) {
    return ErrorMessage(
        absl::StrFormat("Unable to extract time information from log file: %s", log_file_name));
  }
  std::string_view timestamp_string =
      log_file_name.substr(kTimestampStartPos, kTimestampStringLength);
  absl::Time log_file_timestamp;
  std::string parse_time_error;
  if (!absl::ParseTime(kLogFileNameTimeFormat, timestamp_string, absl::UTCTimeZone(),
                       &log_file_timestamp, &parse_time_error)) {
    return ErrorMessage(
        absl::StrFormat("Error while parsing time information from log file %s : %s", log_file_name,
                        parse_time_error));
  }
  return log_file_timestamp;
}

std::vector<std::filesystem::path> FindOldLogFiles(
    absl::Span<const std::filesystem::path> file_paths) {
  std::vector<std::filesystem::path> old_files;
  absl::Time expiration_time = absl::Now() - kLogFileLifetime;
  for (const std::filesystem::path& log_file_path : file_paths) {
    ErrorMessageOr<absl::Time> timestamp_or_error =
        ParseLogFileTimestamp(log_file_path.filename().string());
    if (timestamp_or_error.has_error()) {
      ORBIT_LOG("Warning: %s", timestamp_or_error.error().message());
      continue;
    }
    if (timestamp_or_error.value() < expiration_time) {
      old_files.push_back(log_file_path);
    }
  }
  return old_files;
}

ErrorMessageOr<void> RemoveFiles(absl::Span<const std::filesystem::path> file_paths) {
  std::string error_message;
  for (const auto& file_path : file_paths) {
    std::error_code file_remove_error;
    if (!std::filesystem::remove(file_path, file_remove_error)) {
      absl::StrAppend(&error_message, "Error while removing ", file_path.filename().string(), ": ",
                      file_remove_error.message(), "\n");
    }
  }
  if (!error_message.empty()) {
    return ErrorMessage(error_message);
  }
  return outcome::success();
}

}  // namespace orbit_base_internal
