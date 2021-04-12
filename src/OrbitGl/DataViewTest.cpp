// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>

#include "DataView.h"

using orbit_gl::FormatValueForCsv;

TEST(DataView, FormatValueForCsvQuotesEmptyString) { EXPECT_EQ("\"\"", FormatValueForCsv("")); }

TEST(DataView, FormatValueForCsvQuotesString) {
  EXPECT_EQ("\"string\"", FormatValueForCsv("string"));
}

TEST(DataView, FormatValueForCsvEscapesQuotesInString) {
  constexpr std::string_view kInput("string\"with\"quotes");
  constexpr std::string_view kExpectedResult("\"string\"\"with\"\"quotes\"");
  EXPECT_EQ(kExpectedResult, FormatValueForCsv(kInput));
}
