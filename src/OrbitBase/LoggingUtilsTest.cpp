// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/time/civil_time.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <vector>

#include "LoggingUtils.h"
#include "OrbitBase/Result.h"

namespace orbit_base_internal {
namespace {
std::filesystem::path GenerateTestLogFilePath(const absl::Time& timestamp) {
  std::filesystem::path test_log_dir = "C:/OrbitAppDataDir/logs";
  uint32_t test_pid = 12345;
  std::string timestamp_string =
      absl::FormatTime(kLogFileNameTimeFormat, timestamp, absl::LocalTimeZone());
  std::string filename = absl::StrFormat(kLogFileNameDelimiter, timestamp_string, test_pid);
  return test_log_dir / filename;
}
}  // namespace

TEST(LoggingUtils, ParseLogFileTimestamp) {
  const std::string filename_invalid_no_timestamp = "sfsdf-.log ";
  const std::string filename_invalid_valid_timestamp_wrong_format =
      "Orbitfoobar-2021_01_31_00_00_00-.log";
  const std::string filename_valid = "Orbit-2021_01_31_10_21_33-7188.log";

  {
    ErrorMessageOr<absl::Time> timestamp_or_error =
        ParseLogFileTimestamp(filename_invalid_no_timestamp);
    ASSERT_TRUE(timestamp_or_error.has_error());
    EXPECT_EQ(timestamp_or_error.error().message(),
              absl::StrFormat("Unable to extract time information from log file: %s",
                              filename_invalid_no_timestamp));
  }

  {
    ErrorMessageOr<absl::Time> timestamp_or_error =
        ParseLogFileTimestamp(filename_invalid_valid_timestamp_wrong_format);
    ASSERT_TRUE(timestamp_or_error.has_error());
    EXPECT_THAT(
        timestamp_or_error.error().message(),
        testing::HasSubstr(absl::StrFormat("Error while parsing time information from log file %s",
                                           filename_invalid_valid_timestamp_wrong_format)));
  }

  {
    ErrorMessageOr<absl::Time> timestamp_or_error = ParseLogFileTimestamp(filename_valid);
    absl::Time expected_result =
        absl::FromCivil(absl::CivilSecond(2021, 1, 31, 10, 21, 33), absl::UTCTimeZone());
    ASSERT_FALSE(timestamp_or_error.has_error()) << timestamp_or_error.error().message();
    EXPECT_EQ(timestamp_or_error.value(), expected_result);
  }
}

TEST(LoggingUtils, FindOldLogFiles) {
  absl::Time now = absl::Now();
  std::filesystem::path recent_file = GenerateTestLogFilePath(now - absl::Hours(24));
  std::filesystem::path old_file = GenerateTestLogFilePath(now - absl::Hours(24 * 14));

  std::vector<std::filesystem::path> test_case({recent_file, old_file});
  std::vector<std::filesystem::path> result = FindOldLogFiles(test_case);
  std::vector<std::filesystem::path> expected_result({old_file});
  EXPECT_EQ(result, expected_result);
}

}  // namespace orbit_base_internal