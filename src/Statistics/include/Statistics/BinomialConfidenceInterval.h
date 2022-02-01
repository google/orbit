// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>

namespace orbit_statistics {

struct BinomialConfidenceInterval {
  float lower;
  float upper;
};

class BinomialConfidenceIntervalEstimator {
 public:
  [[nodiscard]] virtual BinomialConfidenceInterval Estimate(float ratio, uint32_t trials) const = 0;
  virtual ~BinomialConfidenceIntervalEstimator() = default;
};

// Estimates 95% binomial confidence intervals using Wilson method.
class WilsonBinomialConfidenceIntervalEstimator : public BinomialConfidenceIntervalEstimator {
 public:
  [[nodiscard]] BinomialConfidenceInterval Estimate(float ratio, uint32_t trials) const override;

 private:
  // (1 - 0.05/2)-quantile of the standard normal distribution.
  constexpr static double kNormalQuantile = 1.959963985;
  constexpr static double kNormalQuantile2 = kNormalQuantile * kNormalQuantile;
};

}  // namespace orbit_statistics