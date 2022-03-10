// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <algorithm>

#include "HistogramWidget.h"

namespace orbit_histogram_widget {
static void TestGetBinWidths(int number_of_bins, int histogram_width) {
  const std::vector<int> widths = GenerageBinWidth(number_of_bins, histogram_width);
  ASSERT_EQ(widths.size(), number_of_bins);
  int sum = std::reduce(std::begin(widths), std::end(widths), 0);
  ASSERT_EQ(sum, histogram_width);
  int max = *std::max_element(std::begin(widths), std::end(widths));
  int min = *std::min_element(std::begin(widths), std::end(widths));
  ASSERT_LE(max - min, 1);
  ASSERT_THAT(widths, testing::Each(testing::Ge(0)));
}

TEST(HistogramUtil, GetBinWidthIsCorrect) {
  TestGetBinWidths(10, 100);
  TestGetBinWidths(10, 115);
  TestGetBinWidths(1, 115);
  TestGetBinWidths(10, 2);
  TestGetBinWidths(1, 1);
}
}  // namespace orbit_histogram_widget