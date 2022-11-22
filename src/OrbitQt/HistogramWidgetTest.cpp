// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

#include "OrbitQt/HistogramWidget.h"

namespace orbit_qt {
static void TestGenerateHistogramBinWidths(int number_of_bins, int histogram_width) {
  const std::vector<int> widths = GenerateHistogramBinWidths(number_of_bins, histogram_width);
  ASSERT_EQ(widths.size(), number_of_bins);
  int sum = std::reduce(std::begin(widths), std::end(widths), 0);
  ASSERT_EQ(sum, histogram_width);
  int max = *std::max_element(std::begin(widths), std::end(widths));
  int min = *std::min_element(std::begin(widths), std::end(widths));
  ASSERT_LE(max - min, 1);
  ASSERT_THAT(widths, testing::Each(testing::Ge(0)));
}

TEST(HistrogramWidget, GenerateHistogramBinWidthsIsCorrect) {
  TestGenerateHistogramBinWidths(10, 100);
  TestGenerateHistogramBinWidths(10, 115);
  TestGenerateHistogramBinWidths(1, 115);
  TestGenerateHistogramBinWidths(10, 2);
  TestGenerateHistogramBinWidths(1, 1);
}
}  // namespace orbit_qt