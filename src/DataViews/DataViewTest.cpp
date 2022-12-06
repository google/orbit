// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>

#include "DataViewTestUtils.h"
#include "DataViews/DataView.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

using orbit_data_views::FormatValueForCsv;

namespace {
constexpr uint64_t kValuesNum = 5;
const std::array<std::string, kValuesNum> kValues = {"a", "b", "c", "d", "e"};
const std::array<std::string, kValuesNum> kCsvFormattedValues = [] {
  std::array<std::string, kValuesNum> result;
  std::transform(std::begin(kValues), std::end(kValues), std::begin(result),
                 [](std::string_view value) { return FormatValueForCsv(value); });
  return result;
}();

const std::string kExpectedFileContent =
    absl::StrCat(absl::StrJoin(kCsvFormattedValues, orbit_data_views::kFieldSeparator),
                 orbit_data_views::kLineSeparator);
}  // namespace

TEST(DataView, FormatValueForCsvQuotesEmptyString) { EXPECT_EQ("\"\"", FormatValueForCsv("")); }

TEST(DataView, FormatValueForCsvQuotesString) {
  EXPECT_EQ("\"string\"", FormatValueForCsv("string"));
}

TEST(DataView, FormatValueForCsvEscapesQuotesInString) {
  constexpr std::string_view kInput("string\"with\"quotes");
  constexpr std::string_view kExpectedResult("\"string\"\"with\"\"quotes\"");
  EXPECT_EQ(kExpectedResult, FormatValueForCsv(kInput));
}

TEST(DataView, WriteLineToCsvIsCorrect) {
  orbit_test_utils::TemporaryFile temporary_file = orbit_data_views::GetTemporaryFilePath();
  EXPECT_THAT(orbit_data_views::WriteLineToCsv(temporary_file.fd(), kValues),
              orbit_test_utils::HasNoError());

  ErrorMessageOr<std::string> contents_or_error =
      orbit_base::ReadFileToString(temporary_file.file_path());
  EXPECT_THAT(contents_or_error, orbit_test_utils::HasNoError());

  EXPECT_EQ(contents_or_error.value(), kExpectedFileContent);
}