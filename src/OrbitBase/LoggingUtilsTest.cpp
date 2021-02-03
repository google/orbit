// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/time/clock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "LoggingUtils.h"

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

namespace orbit_base {

TEST(LoggingUtils, GetLogFileLifetime) {
  const std::string kFilenameInvalidNoTimestamp = "sfsdf-.log ";
  const std::string kFilenameInvalidValidTimestampWrongFormat =
      "Orbitfoobar-2021_01_31_00_00_00-.log";
  const std::string kFilenameValid = "Orbit-2021_01_31_00_00_00-7188.log";

  ErrorMessageOr<absl::Duration> result_extract_failed =
      GetLogFileLifetime(kFilenameInvalidNoTimestamp);
  ASSERT_FALSE(result_extract_failed);
  EXPECT_EQ(result_extract_failed.error().message(),
            absl::StrFormat("Unable to extract time information from log file: %s",
                            kFilenameInvalidNoTimestamp));

  ErrorMessageOr<absl::Duration> result_parse_failed =
      GetLogFileLifetime(kFilenameInvalidValidTimestampWrongFormat);
  ASSERT_FALSE(result_parse_failed);
  EXPECT_THAT(
      result_parse_failed.error().message(),
      testing::HasSubstr(absl::StrFormat("Error while parsing time information from log file %s",
                                         kFilenameInvalidValidTimestampWrongFormat)));

  ErrorMessageOr<absl::Duration> result_parse_succeed = GetLogFileLifetime(kFilenameValid);
  absl::Duration expected_result =
      absl::Now() - absl::FromCivil(absl::CivilSecond(2021, 1, 31, 0, 0, 0), absl::LocalTimeZone());
  EXPECT_NEAR(absl::ToDoubleSeconds(result_parse_succeed.value()),
              absl::ToDoubleSeconds(expected_result), 1);
}

TEST(LoggingUtils, FilterOldLogFiles) {
  absl::Time now = absl::Now();
  std::filesystem::path recent_file = GenerateTestLogFilePath(now - absl::Hours(24));
  std::filesystem::path old_file = GenerateTestLogFilePath(now - absl::Hours(24 * 14));

  std::vector<std::filesystem::path> test_case({recent_file, old_file});
  std::vector<std::filesystem::path> result = FilterOldLogFiles(test_case);
  std::vector<std::filesystem::path> expected_result({old_file});
  EXPECT_EQ(result, expected_result);
}

}  // namespace orbit_base