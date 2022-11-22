// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_HISTOGRAM_H_
#define STATISTICS_HISTOGRAM_H_

#include <absl/types/span.h>
#include <stddef.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace orbit_statistics {

// Represents the inclusive range the user has selected on the HistogramWidget.
struct HistogramSelectionRange {
  uint64_t min_duration;
  uint64_t max_duration;
};

// A histogram of a dataset of `uint64_t` values with bins of equal width (with a possible
// exception for the last one). The bins are:
// [min, min + bin_width), [min + bin_width, min + 2*bin_width), ...
// ... [min + (counts*size() - 1) * bin_width, max]
// `counts[i]` stores the number of elements in i-th bin.
struct Histogram {
  uint64_t min{};
  uint64_t max{};
  uint64_t bin_width{};
  size_t data_set_size{};
  std::vector<size_t> counts;
};

// The function builds multiple histograms with different number of bins,
// estimates the risk score using `HistogramRiskScore` and returns the histogram
// which minimizes it. The histogram will not own the data.
[[nodiscard]] std::optional<Histogram> BuildHistogram(absl::Span<const uint64_t> data);

}  // namespace orbit_statistics

#endif  // STATISTICS_HISTOGRAM_H_