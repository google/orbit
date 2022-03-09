// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/Histogram.h"

#include <absl/types/span.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "HistogramUtils.h"
#include "OrbitBase/Logging.h"
#include "Statistics/DataSet.h"

namespace orbit_statistics {

// These values are chosen to be arbitrarily large/small enough
constexpr uint32_t kNumberOfBinsGridSize = 12;
constexpr size_t kLargeNumberOfBins = 2048;  // 2^11, not necessarily 2^(kNumberOfBinsGridSize-1)
constexpr uint32_t kVeryLargeDatasetThreshold = 10'000'000;

static Histogram BuildHistogramWithNumberOfBins(const std::optional<DataSet>& data_set,
                                                size_t number_of_bins) {
  uint64_t bin_width = NumberOfBinsToBinWidth(data_set.value(), number_of_bins);
  return BuildHistogram(data_set.value(), bin_width);
}

[[nodiscard]] std::optional<Histogram> BuildHistogram(absl::Span<const uint64_t> data) {
  std::optional<DataSet> data_set = DataSet::Create(data);
  if (!data_set.has_value()) return std::nullopt;

  // if the data set is extremely large, we surely have enough data
  // to populate the maximal number of bins.
  if (data_set->GetData().size() > kVeryLargeDatasetThreshold) {
    return BuildHistogramWithNumberOfBins(data_set, kLargeNumberOfBins);
  }

  size_t number_of_bins = 1;
  double best_risk_score = std::numeric_limits<double>::max();
  Histogram best_histogram;

  for (uint32_t i = 0; i < kNumberOfBinsGridSize; ++i) {
    Histogram histogram = BuildHistogramWithNumberOfBins(data_set, number_of_bins);
    double risk_score = HistogramRiskScore(histogram);
    if (risk_score < best_risk_score) {
      best_risk_score = risk_score;
      best_histogram = std::move(histogram);
    }
    number_of_bins *= 2;
  }

  return best_histogram;
}

[[nodiscard]] std::vector<int> GetBinWidth(size_t number_of_bins, int histogram_width) {
  const int narrower_width = histogram_width / number_of_bins;
  const int wider_width = narrower_width + 1;

  const int number_of_wider_bins = histogram_width % number_of_bins;
  const int number_of_narrower_bins = number_of_bins - number_of_wider_bins;

  std::vector<int> result(number_of_narrower_bins, narrower_width);
  const std::vector<int> wider_widths(number_of_wider_bins, wider_width);
  result.insert(std::end(result), std::begin(wider_widths), std::end(wider_widths));

  std::shuffle(std::begin(result), std::end(result), std::default_random_engine{});
  return result;
}

}  // namespace orbit_statistics