// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_HISTOGRAM_UTILS_H_
#define STATISTICS_HISTOGRAM_UTILS_H_

#include <stddef.h>

#include <cstdint>

#include "Statistics/DataSet.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

// The function estimates risk score (integral of squared difference of the histogram and true
// distribution). Implements the analytical expression for the leave-one-out estimate. The lower --
// the better.
// If the data set happens to be singular (histogram.min == histogram.max), 0.0 is returned.
[[nodiscard]] double HistogramRiskScore(const Histogram& histogram);

[[nodiscard]] size_t ValueToHistogramBinIndex(uint64_t value, const DataSet& data_set,
                                              uint64_t bin_width);

[[nodiscard]] uint64_t NumberOfBinsToBinWidth(const DataSet& data_set, size_t bins_num);

[[nodiscard]] Histogram BuildHistogram(const DataSet& data_set, uint64_t bin_width);

}  // namespace orbit_statistics

#endif /* STATISTICS_HISTOGRAM_UTILS_H_ */
