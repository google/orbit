// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_BINOMIAL_CONFIDENCE_INTERVAL_H_
#define STATISTICS_BINOMIAL_CONFIDENCE_INTERVAL_H_

#include <cstdint>

namespace orbit_statistics {

struct BinomialConfidenceInterval {
  float lower;
  float upper;
};

// Estimates the Binomial proportion confidence interval given the proportion of successful trials
// and the total number of trials
class BinomialConfidenceIntervalEstimator {
 public:
  // `ratio` is the proportion of successful trials and should be between 0 and 1.
  [[nodiscard]] virtual BinomialConfidenceInterval Estimate(float ratio, uint32_t trials) const = 0;
  virtual ~BinomialConfidenceIntervalEstimator() = default;
};

// Estimates 95% binomial confidence intervals using Wilson method.
class WilsonBinomialConfidenceIntervalEstimator : public BinomialConfidenceIntervalEstimator {
 public:
  [[nodiscard]] BinomialConfidenceInterval Estimate(float ratio, uint32_t trials) const override;
};

}  // namespace orbit_statistics

#endif  // STATISTICS_BINOMIAL_CONFIDENCE_INTERVAL_H_