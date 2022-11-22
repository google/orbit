// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

#include "Statistics/BinomialConfidenceInterval.h"

namespace orbit_statistics {

static void ExpectIntervalEq(const BinomialConfidenceInterval& interval, float expected_lower,
                             float expected_upper) {
  EXPECT_FLOAT_EQ(interval.lower, expected_lower);
  EXPECT_FLOAT_EQ(interval.upper, expected_upper);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ZeroSampleSizeTest) {
  const WilsonBinomialConfidenceIntervalEstimator estimator;
  for (float ratio : {0.0f, 0.2f, 0.8f, 1.0f}) {
    auto interval = estimator.Estimate(ratio, 0);
    ExpectIntervalEq(interval, 0.0f, 1.0f);
  }
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, LargeSampleSmallProbabilityTest) {
  const WilsonBinomialConfidenceIntervalEstimator estimator;
  auto interval = estimator.Estimate(0.025f, 2800);
  ExpectIntervalEq(interval, 0.01983537f, 0.03146619f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleSmallProbabilityTest) {
  const WilsonBinomialConfidenceIntervalEstimator estimator;
  auto interval = estimator.Estimate(0.1f, 10);
  ExpectIntervalEq(interval, 0.01787621f, 0.40415f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleLargeProbabilityTest) {
  const WilsonBinomialConfidenceIntervalEstimator estimator;
  auto interval = estimator.Estimate(0.9f, 10);
  ExpectIntervalEq(interval, 0.59585f, 0.9821238f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleLargeZeroTest) {
  const WilsonBinomialConfidenceIntervalEstimator estimator;
  auto interval = estimator.Estimate(0.0f, 10);
  ExpectIntervalEq(interval, 0.0f, 0.2775328f);
}

}  // namespace orbit_statistics