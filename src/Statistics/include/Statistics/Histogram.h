// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_HISTOGRAM_H_
#define STATISTICS_HISTOGRAM_H_

#include <cstdint>
#include <optional>
#include <vector>

namespace orbit_statistics {

// A histogram of a dataset of `uint64_t` values with bins of equal width (with a possible
// exception for the last one). The bins are:
// [min, min + bandwidth), [min + bandwidth, min + 2*bandwidth), ...
// ... [min + (counts*size() - 1) * bandwidth, max]
// `counts[i]` stores the number of elements in i-th bin.
struct Histogram {
  uint64_t min{};
  uint64_t max{};
  uint64_t bandwidth{};
  size_t data_set_size{};
  std::vector<size_t> counts;
};

// The function builds multiple histograms with different number of bins,
// estimates the risk score using `HistogramRiskScore` and returns the histogram
// which minimizes it.
[[nodiscard]] std::optional<Histogram> BuildHistogram(const std::vector<uint64_t>& data);
}  // namespace orbit_statistics

#endif  // STATISTICS_HISTOGRAM_H_