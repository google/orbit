// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_join.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <numeric>
#include <string>
#include <string_view>

#include "DataViewTestUtils.h"
#include "DataViews/DataView.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

using orbit_data_views::FormatValueForCsv;

TEST(DataView, FormatValueForCsvQuotesEmptyString) { EXPECT_EQ("\"\"", FormatValueForCsv("")); }

TEST(DataView, FormatValueForCsvQuotesString) {
  EXPECT_EQ("\"string\"", FormatValueForCsv("string"));
}

TEST(DataView, FormatValueForCsvEscapesQuotesInString) {
  constexpr std::string_view kInput("string\"with\"quotes");
  constexpr std::string_view kExpectedResult("\"string\"\"with\"\"quotes\"");
  EXPECT_EQ(kExpectedResult, FormatValueForCsv(kInput));
}

constexpr uint64_t kValuesNum = 5;
const std::array<std::string, kValuesNum> kValues = {"a", "b", "c", "d", "e"};
const std::array<std::string, kValuesNum> kCSVFormattedValues = [] {
  std::array<std::string, kValuesNum> result;
  std::transform(std::begin(kValues), std::end(kValues), std::begin(result),
                 [](const std::string& value) { return FormatValueForCsv(value); });
  return result;
}();

const std::string kExpectedFileContent =
    absl::StrCat(absl::StrJoin(kCSVFormattedValues, orbit_data_views::kFieldSeparator),
                 orbit_data_views::kLineSeparator);

TEST(DataView, WriteLineToCsvIsCorrect) {
  orbit_base::TemporaryFile temporary_file = orbit_data_views::GetTemporaryFilePath();
  auto file_or_error = orbit_base::OpenFileForWriting(temporary_file.file_path().string());
  EXPECT_THAT(file_or_error, orbit_test_utils::HasNoError());
  EXPECT_THAT(orbit_data_views::WriteLineToCsv(file_or_error.value(), kValues),
              orbit_test_utils::HasNoError());

  ErrorMessageOr<std::string> contents_or_error =
      orbit_base::ReadFileToString(temporary_file.file_path());
  EXPECT_THAT(contents_or_error, orbit_test_utils::HasNoError());

  EXPECT_EQ(contents_or_error.value(), kExpectedFileContent);
}