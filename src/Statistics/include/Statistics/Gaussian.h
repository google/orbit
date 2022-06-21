// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_GAUSSIAN_H_
#define STATISTICS_GAUSSIAN_H_

#include <cmath>

namespace orbit_statistics {

// Cumulative density function of the standard (zero mean, unit variance) Gaussian distribution.
[[nodiscard]] inline double GaussianCdf(double x) { return std::erfc(-x / std::sqrt(2)) / 2; }

}  // namespace orbit_statistics

#endif  // STATISTICS_GAUSSIAN_H_
