// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

#include "Histogram.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

TEST(HistogramTest, RiskScoreTest) {
  const uint64_t bandwidth = 7421300;
  const std::vector<uint64_t> counts = {32ULL, 30ULL, 174ULL, 42ULL, 2ULL};
  const uint64_t min = 14015002;
  const uint64_t max = 43843646;
  const size_t data_set_size = 280;
  Histogram histogram{min, max, bandwidth, data_set_size, counts};
  EXPECT_NEAR(HistogramRiskScore(histogram), -1.72, 0.01);
}

TEST(HistogramTest, RiskScoreTestZeroWidth) {
  Histogram histogram{0, 0, 1, 1, {}};
  EXPECT_DOUBLE_EQ(HistogramRiskScore(histogram), 0.0);
}

}  // namespace orbit_statistics