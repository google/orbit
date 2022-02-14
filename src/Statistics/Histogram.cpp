// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/Histogram.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "HistogramUtils.h"
#include "OrbitBase/Logging.h"
#include "Statistics/DataSet.h"

namespace orbit_statistics {

constexpr uint32_t kNumberOfBinsGridSize = 12;

[[nodiscard]] std::optional<Histogram> BuildHistogram(const std::vector<uint64_t>& data) {
  std::optional<DataSet> data_set = DataSet::Create(&data);
  if (!data_set.has_value()) return std::nullopt;

  size_t number_of_bins = 1;
  double best_risk_score = std::numeric_limits<double>::max();
  Histogram best_histogram;

  for (uint32_t i = 0; i < kNumberOfBinsGridSize; ++i) {
    uint64_t bin_width = NumberOfBinsToBinWidth(data_set.value(), number_of_bins);
    auto histogram = BuildHistogram(data_set.value(), bin_width);
    double risk_score = HistogramRiskScore(histogram);

    if (risk_score < best_risk_score) {
      best_risk_score = risk_score;
      best_histogram = std::move(histogram);
    }
    number_of_bins *= 2;
  }

  return best_histogram;
}

}  // namespace orbit_statistics