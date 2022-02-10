// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_HISTOGRAM_BUILDER_H_
#define STATISTICS_HISTOGRAM_BUILDER_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "Statistics/Histogram.h"

namespace orbit_statistics {

/**
 * The function estimates risk score (integral of squared difference of the histogram and true
 * distribution). Implements the analytical expression for the leave-one-out estimate. The lower --
 * the better.
 * If the data set happens to be singular (histogram.min == histogram.max), 0.0 is returned.
 */
double HistogramRiskScore(const Histogram& histogram);

class HistogramBuilderTest;

/**
 * Should be used to construct a histogram. The intended use is
 * ```
 * HistogramBuilder builder;
 * builder.SetDataSet(data_ptr);
 * builder.SetBandwidth(bandwidth); // alternatively, build.SetNumberOfBins(bin_num);
 * builder.Build();
 * ```
 */
class HistogramBuilder {
 public:
  void SetDataSet(const std::vector<uint64_t>* data);

  void SetBandwidth(uint64_t bandwidth);

  void SetNumberOfBins(size_t bins_num);

  [[nodiscard]] std::unique_ptr<const Histogram> Build() const;

 private:
  friend HistogramBuilderTest;

  [[nodiscard]] size_t ValueToIndex(uint64_t value) const;

  const std::vector<uint64_t>* data_ = {};
  std::optional<uint64_t> max_ = std::nullopt;
  std::optional<uint64_t> min_ = std::nullopt;
  std::optional<uint64_t> bandwidth_ = std::nullopt;
};
}  // namespace orbit_statistics
#endif  // STATISTICS_HISTOGRAM_BUILDER_H_
