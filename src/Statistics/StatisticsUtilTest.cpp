// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "Statistics/StatisticsUtils.h"

namespace orbit_statistics {

using testing::DoubleNear;

static void ExpectMeanAndVarianceEq(const MeanAndVariance& actual,
                                    const MeanAndVariance& expected) {
  constexpr double kTolerance = 1e-3;
  EXPECT_THAT(actual.mean, DoubleNear(expected.mean, kTolerance));
  EXPECT_THAT(actual.variance, DoubleNear(expected.variance, kTolerance));
}

const MeanAndVariance kFirstRV = {2, 3};
const MeanAndVariance kSecondRV = {4, 5};

TEST(StatisticsUtilTest, ProductOfTwoIndependentIsCorrect) {
  MeanAndVariance product = ProductOfTwoIndependent(kFirstRV, kSecondRV);
  MeanAndVariance expected = {8.0, 83.0};
  ExpectMeanAndVarianceEq(product, expected);
}

TEST(StatisticsUtilTest, DiffOfTwoIndependentIsCorrect) {
  MeanAndVariance product = DiffOfTwoIndependent(kFirstRV, kSecondRV);
  MeanAndVariance expected = {-2, 8};
  ExpectMeanAndVarianceEq(product, expected);
}

}  // namespace orbit_statistics