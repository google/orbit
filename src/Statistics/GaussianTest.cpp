// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <array>

#include "Statistics/Gaussian.h"

namespace orbit_statistics {

using testing::DoubleNear;

constexpr size_t kValuesCnt = 3;
constexpr std::array<double, kValuesCnt> kValues = {0, 1.96, 100};
constexpr std::array<double, kValuesCnt> kExpectedCDFValue = {0.5, 1 - 0.05 / 2, 1};

TEST(GaussianTest, GaussianCDFIsCorrect) {
  for (size_t i = 0; i < kValuesCnt; ++i) {
    EXPECT_THAT(GaussianCDF(kValues[i]), DoubleNear(kExpectedCDFValue[i], 1e-3));

    // Due to symmetry of Gaussian distribution
    EXPECT_THAT(GaussianCDF(-kValues[i]), DoubleNear(1 - kExpectedCDFValue[i], 1e-3));
  }
}

}  // namespace orbit_statistics