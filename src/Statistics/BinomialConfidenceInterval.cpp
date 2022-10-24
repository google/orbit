// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/BinomialConfidenceInterval.h"

#include <cmath>

namespace orbit_statistics {

[[nodiscard]] static double constexpr sqr(double x) { return x * x; }

[[nodiscard]] BinomialConfidenceInterval WilsonBinomialConfidenceIntervalEstimator::Estimate(
    float ratio, uint32_t trials) const {
  // (1 - 0.05/2)-quantile of the standard normal distribution.
  constexpr static double kNormalQuantile = 1.959963985;
  constexpr static double kNormalQuantileSqr = sqr(kNormalQuantile);

  if (trials == 0) {
    return {0.0f, 1.0f};
  }

  const double denominator = 1. + kNormalQuantileSqr / trials;
  const double corrected_mean = (ratio + 0.5 * kNormalQuantileSqr / trials) / denominator;
  const double corrected_standard_deviation =
      (std::sqrt(ratio * (1.0 - ratio) / trials + kNormalQuantileSqr / sqr(2 * trials))) /
      denominator;
  const double quantile_times_stddev = kNormalQuantile * corrected_standard_deviation;
  BinomialConfidenceInterval result{static_cast<float>(corrected_mean - quantile_times_stddev),
                                    static_cast<float>(corrected_mean + quantile_times_stddev)};
  return result;
}

}  // namespace orbit_statistics