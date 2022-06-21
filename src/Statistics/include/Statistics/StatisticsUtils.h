// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_STATISTICS_UTILS_H_
#define STATISTICS_STATISTICS_UTILS_H_

#include <algorithm>

#include "Statistics/BinomialConfidenceInterval.h"

namespace orbit_statistics {

// Returns the value to the right from ± in notation like `0.03 ± 0.002`.
// In this notation only the longer section of the confidence interval is shown,
// e. g. for the interval `(0.029; 0.032)` constructed around the `rate == 0.03`
// 0.002 is returned.
[[nodiscard]] inline float HalfWidthOfSymmetrizedConfidenceInterval(
    const BinomialConfidenceInterval& interval, const float rate) {
  return std::max(interval.upper - rate, rate - interval.lower);
}

struct MeanAndVariance {
  [[nodiscard]] double SecondMoment() const { return mean * mean + variance; }

  double mean;
  double variance;
};

[[nodiscard]] inline MeanAndVariance ProductOfTwoIndependent(const MeanAndVariance& x,
                                                             const MeanAndVariance& y) {
  const double product_of_means = x.mean * y.mean;
  return {product_of_means,
          x.SecondMoment() * y.SecondMoment() - product_of_means * product_of_means};
}

[[nodiscard]] inline MeanAndVariance DiffOfTwoIndependent(const MeanAndVariance& x,
                                                          const MeanAndVariance& y) {
  return {x.mean - y.mean, x.variance + y.variance};
}

}  // namespace orbit_statistics

#endif  // STATISTICS_STATISTICS_UTILS_H_
