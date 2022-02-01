// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/BinomialConfidenceInterval.h"

#include <cmath>
#include <cstdint>

namespace orbit_statistics {

double sqr(double x) { return x * x; }

[[nodiscard]] BinomialConfidenceInterval WilsonBinomialConfidenceIntervalEstimator::Estimate(
    float ratio, uint32_t trials) const {
  if (trials == 0) {
    return {0.0f, 1.0f};
  }

  const double denominator = 1. + kNormalQuantile2 / trials;
  const double corrected_mean = (ratio + 0.5 * kNormalQuantile2 / trials) / denominator;
  const double corrected_standard_deviation =
      (std::sqrt(ratio * (1.0 - ratio) / trials + kNormalQuantile2 / sqr(2 * trials))) /
      denominator;
  const double quantile_times_stddev = kNormalQuantile * corrected_standard_deviation;
  BinomialConfidenceInterval result{static_cast<float>(corrected_mean - quantile_times_stddev),
                                    static_cast<float>(corrected_mean + quantile_times_stddev)};
  return result;
}

}  // namespace orbit_statistics