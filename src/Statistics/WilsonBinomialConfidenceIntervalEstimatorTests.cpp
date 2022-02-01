// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include <gtest/gtest.h>

#include "Statistics/BinomialConfidenceInterval.h"


namespace orbit_statistics {

const WilsonBinomialConfidenceIntervalEstimator kEstimator;

void ExpectIntervalEQ(const BinomialConfidenceInterval& interval, float expected_lower, float expected_upper) {
  EXPECT_FLOAT_EQ(interval.lower, expected_lower);
  EXPECT_FLOAT_EQ(interval.upper, expected_upper);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ZeroSampleSizeTest) {
  for (float ratio: {0.0f, 0.2f, 0.8f, 1.0f}) {
    auto interval = kEstimator.Estimate(ratio, 0); 
    ExpectIntervalEQ(interval, 0.0f, 1.0f);
  }
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, LargeSampleSmallProbabilityTest) {
  auto interval = kEstimator.Estimate(0.025, 2800); 
  ExpectIntervalEQ(interval, 0.01983537f, 0.03146619f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleSmallProbabilityTest) {
  auto interval = kEstimator.Estimate(0.1, 10); 
  ExpectIntervalEQ(interval, 0.01787621f, 0.40415f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleLargeProbabilityTest) {
  auto interval = kEstimator.Estimate(0.9, 10); 
  ExpectIntervalEQ(interval, 0.59585f, 0.9821238f);
}

TEST(WilsonBinomialConfidenceIntervalEstimatorTest, ShortSampleLargeZeroTest) {
  auto interval = kEstimator.Estimate(0.0, 10); 
  ExpectIntervalEQ(interval, 0.0f, 0.2775328f);
}

}